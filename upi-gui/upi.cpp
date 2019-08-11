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

void UPIManager::upi(PipeManager& p) {
    p.sendMessage("upi");
}

EngineInfo UPIManager::id(PipeManager& p) {
    EngineInfo ei;
    try {
        std::string recv;

        if (p.recvMessage(recv) == -1) {
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

        ei.engine_path = p.processPath();
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

void UPIManager::tumo(PipeManager& p, const Tumo* tumo) {
    p.sendMessage("tumo " + getTumo128ToString(tumo));
}

void UPIManager::position(PipeManager& p, Field& self, Field& enemy) {
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
    int unconformed_ojama = self_clone.getConformedOjama() - enemy_clone.getConformedOjama();

    std::stringstream ss;
    ss << "position " << self.toPfen() << " " << enemy.toPfen() << " " << conformed_ojama << " " << unconformed_ojama << " " << frame;
    p.sendMessage(ss.str());
}

void UPIManager::isready(PipeManager& p) {
    p.sendMessage("isready");
}

void UPIManager::go(PipeManager& p) {
    p.sendMessage("go");
}

void UPIManager::quit(PipeManager& p) {
    p.sendMessage("quit");
    p.closeProcess();
}

void UPIManager::gameover(PipeManager& p, bool win) {
    std::string win_string = (win ? "win" : "lose");
    p.sendMessage("gameover " + win_string);
}

Move UPIManager::bestmove(PipeManager& p, Field& field) {
    std::string bestmove, dummy, move;
    p.recvMessage(bestmove);
    std::istringstream iss(bestmove);
    iss >> dummy >> move;
    
    if (dummy != "bestmove") {
        return MOVE_NONE;
    }

    deleteCRLF(move);
    Move m = upiToMove(move, field);

    if (m == MOVE_NONE) {
        p.sendMessage("pfen");
        std::string pfen;
        p.recvMessage(pfen);
        Log::write("Illegal move: " + move + ", pfen = " + pfen + "\n");
        Move mm = upiToMove(move, field);
    }

    return m;
}

void UPIManager::setEngineMove(PipeManager& p, Field& self, Field& enemy, OperationQueue& queue) {
    position(p, self, enemy);
    go(p);
    Move move = bestmove(p, self);

    if (move == MOVE_NONE) {
        return;
    }

    queue.moveToOperation(move, self);

    Field self_clone(self), enemy_clone(enemy);

    // 今この場でその操作通りにおいてみて、おけなかったらやばい
    auto ope_clone = queue;
    auto tumo = self_clone.getTumo();

    while (self_clone.getGamefase() != CHECK_VANISH) {
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
}

void UPIManager::launchEngine(PipeManager& p, const Tumo* tumo) {
    p.sendMessage("tumo " + getTumo128ToString(tumo));
    p.sendMessage("isready");
    std::string r;
    do {
        p.recvMessage(r);
        deleteCRLF(r);
    } while (r != "readyok");
}