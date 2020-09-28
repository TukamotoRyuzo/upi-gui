#include "upi.h"
#include "move.h"
#include "main_window.h"

#include <sstream>

std::string UPIManager::getTumo128ToString(const Tumo* tumo) const {
    std::stringstream ss;

    for (int i = 0; i < TUMO_MAX; i++) {
        ss << colorToString(tumo[i].color[0]) << colorToString(tumo[i].color[1]);
        if (i != TUMO_MAX - 1) {
            ss << " ";
        }
    }

    return ss.str();
}

void UPIManager::upi() {
    pipe.sendMessage("upi");
}

EngineInfo UPIManager::id() {
    EngineInfo ei;
    try {
        std::string recv;

        if (pipe.recvMessage(recv) == -1) {
            throw std::runtime_error("upiエンジンではありません。");
        }

        std::istringstream ss(recv);
        std::string engine_name = "", author = "";

        auto check = [](std::istringstream& ss, std::string expected) {
            std::string tmp;
            ss >> tmp;
            if (tmp != expected) {
                throw std::runtime_error("upiエンジンではありません。");
            }
        };

        auto extract_value = [](std::istringstream& ss_in) {
            std::string line;
            std::getline(ss_in, line);
            deleteCRLF(line);
            return line;
        };

        ei.engine_path = pipe.processPath();
        check(ss, "id");
        check(ss, "name");
        ei.engine_name = extract_value(ss);
        check(ss, "id");
        check(ss, "author");
        ei.engine_author = extract_value(ss);
        check(ss, "upiok");
    }
    catch (std::exception& ex) {
        Log::write(ex.what());
    }

    return ei;
}

void UPIManager::tumo(const Tumo* tumo) {
    pipe.sendMessage("tumo " + getTumo128ToString(tumo));
}

void UPIManager::rule(Rule& r) {
    std::ostringstream ss;
    ss << "rule"
        << " falltime " << r.fall_time
        << " chaintime " << r.chain_time
        << " settime " << r.set_time
        << " nexttime " << r.next_time
        << " autodroptime " << r.autodrop_time;
    pipe.sendMessage(ss.str());
}

void UPIManager::position(Field& self, Field& enemy) {
    // 相手が行動可能になるまでのフレーム数を計算する
    Field self_clone(self), enemy_clone(enemy);
    int frame = 0;

    if (enemy_clone.getGamefase() != OPERATION) {
        while (enemy_clone.getGamefase() != NEXT
            && enemy_clone.getGamefase() != GAMEOVER) {
            enemy_clone.update(self_clone, NO_OPERATION);
            frame++;
        }
    }

    // ゲームオーバーなら思考する必要なし
    if (enemy_clone.getGamefase() == GAMEOVER) {
        return;
    }

    // 予告ぷよを取得
    int conformed_ojama = self.getConformedOjama() - enemy.getConformedOjama();
    int unconformed_ojama = frame ? (self_clone.getConformedOjama() - enemy_clone.getConformedOjama())
                                  : (self.getUnconformedOjama() - enemy.getUnconformedOjama());

    if (conformed_ojama < 0 && unconformed_ojama > 0 || conformed_ojama > 0 && unconformed_ojama < 0)
    {
        conformed_ojama = 0;
    }
    else if (frame && conformed_ojama < 0 && unconformed_ojama <= 0)
    {
        conformed_ojama = unconformed_ojama;
        unconformed_ojama = 0;
    }

    std::stringstream ss;
    ss << "position " << self.toPfen() << " " << enemy_clone.toPfen() << " " << conformed_ojama << " " << unconformed_ojama << " " << frame;
    pipe.sendMessage(ss.str());
}

void UPIManager::isready() {
    pipe.sendMessage("isready");
}

void UPIManager::go() {
    pipe.sendMessage("go");
}

void UPIManager::quit() {
    pipe.sendMessage("quit");
    pipe.closeProcess();
}

void UPIManager::gameover(bool win) {
    std::string win_string = (win ? "win" : "lose");
    pipe.sendMessage("gameover " + win_string);
}

void UPIManager::illegalmove(Move move) {
    pipe.sendMessage("illegalmove " + toUPI(move));
}

Move UPIManager::bestmove(Field& field) {
    std::string bestmove, dummy, move;
    pipe.recvMessage(bestmove);
    std::istringstream iss(bestmove);
    iss >> dummy >> move;
    
    if (dummy != "bestmove") {
        return MOVE_NONE;
    }

    deleteCRLF(move);
    return upiToMove(move, field);
}

int getMoveFrame(Move m, Rule r, int chain)
{
    Square p = psq(m), c = csq(m);
    int frame = 0;
    int rotate = 0;

    int tateframe = 0;

    // 縦移動
    if (fileOf(p) == fileOf(c))
    {
        if (rankOf(p) < rankOf(c))
        {
            rotate = 0;
        }
        else
        {
            rotate = 2;
        }

        tateframe = (12 - rankOf(p)) * r.fall_time;
    }
    else
    {
        rotate = 1;
        tateframe = std::max(12 - rankOf(p), 12 - rankOf(c)) * r.fall_time;
    }

    if (tateframe)
        tateframe -= 1;

    // 横移動
    int yoko = abs(2 - fileOf(p));
    int yokoframe = yoko * r.fall_time;

    if (yokoframe)
        yokoframe -= 1;

    int rotateframe = 0;

    // 回転数
    if (yoko < rotate)
    {
        rotateframe = (rotate - yoko) * r.fall_time;
        if (!yokoframe)
            rotateframe -= 1;
    }

    // operation
    frame += tateframe;
    frame += yokoframe;
    frame += rotateframe;

    // set_time
    frame += isTigiri(m) ? r.set_time * 2 : r.set_time;

    // check_slide
    frame += r.chain_time * chain + chain;

    // check_vanish
    frame += (r.set_time + 1) * chain;
    
    // chain_voice
    frame += 1 * (chain + 1);
    
    frame += r.next_time;

    // chain
    frame += 5;

    return frame;
}

bool UPIManager::setEngineMove(Field& self, Field& enemy, OperationQueue& queue) {
    Move move = bestmove(self);

    if (move == MOVE_NONE) {
        illegalmove(move);
        return false;
    }

    queue.moveToOperationSoft(move, self);
#ifdef _DEBUG
    Field self_clone(self), enemy_clone(enemy);

    // 今この場でその操作通りにおいてみて、おけなかったらやばい
    auto ope_clone = queue;
    auto tumo = self_clone.getTumo();
    int frame = 0;
    int preexpected = getMoveFrame(move, *self.getRule(), 0);
    int frames[GAMEOVER + 1] = { 0 };
    GameFase fase;
    while ((fase = self_clone.getGamefase()) != CHECK_VANISH) {
        frames[fase]++;
        frame++;
        self_clone.update(enemy_clone, ope_clone.size() ? ope_clone.pop() : NO_OPERATION);
    }

    auto pcol = self_clone.color(fileOf(psq(move)) + 1, rankOf(psq(move)) + 1);
    auto ccol = self_clone.color(fileOf(csq(move)) + 1, rankOf(csq(move)) + 1);

    if (tumo.color[0] != pcol || tumo.color[1] != ccol) {
        std::stringstream ss;
        ss << "Illegal operation: " << move
            << fileOf(psq(move)) << " " << rankOf(psq(move)) << " "
            << fileOf(csq(move)) << " " << rankOf(csq(move)) << std::endl;
        Log::write(ss.str());
    }

    int chainmax = 0;
    while ((fase = self_clone.getGamefase()) != OPERATION
        && fase != GAMEOVER) {
        frames[fase]++;
        frame++;
        self_clone.update(enemy_clone, NO_OPERATION);
        chainmax = std::max(self_clone.getChain(), chainmax);
    }
    std::stringstream ss;
    int expected = getMoveFrame(move, *self.getRule(), chainmax);

    if (expected != frame)
    {
        OperationQueue oq;
        oq.moveToOperationSoft(move, self);
        ss << "expected frame = " << expected << " actual frame = " << frame << std::endl;
    }
    expected = getMoveFrame(move, *self.getRule(), chainmax);
    
    Field self_clone2(self);
    auto ope_clone2 = queue;
    int ff = 0;
    while ((fase = self_clone2.getGamefase()) != CHECK_VANISH) {
        ff++;
        self_clone2.update(enemy_clone, ope_clone2.size() ? ope_clone2.pop() : NO_OPERATION);
    }


    Log::write(ss.str());
#endif

    return true;
}

void UPIManager::launchEngine(const Tumo* t, Rule& r) {
    tumo(t);
    rule(r);
    isready();
    std::string readyok;
    do {
        pipe.recvMessage(readyok);
        deleteCRLF(readyok);
    } while (readyok != "readyok");
}

// クライアントからのメッセージを待ち受けるスレッド。
void UPIManager::doRecvLoop() {
    active = true;

    while (active) {
        std::string s;

        if (pipe.recvMessage(s)) {
            deleteCRLF(s);

            std::stringstream ss(s);
            std::string tmp = "";
            ss >> tmp;

            if (s == "readyok") {
                // 何もしない
            }
            else if (s == "bestmove") {

            }
            else if (s == "id") {

            }
            else {
                // 思考エンジンにエラーメッセージを送り返す
                pipe.sendMessage("404");
            }
        }
    }
}
