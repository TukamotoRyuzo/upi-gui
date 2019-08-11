#include "battle_history.h"
#include "game.h"

void BattleHistory::clear() {
    rule = Rule();
    player_1p_name.clear();
    player_2p_name.clear();
    move_history_1p.clear();
    move_history_2p.clear();
    tumo_history.clear();
}

// ƒŠƒvƒŒƒCî•ñ‚Ì‰Šú‰»
void BattleHistory::init(Rule r, std::string p1_name, std::string p2_name, Tumo* tumo) {
    clear();
    rule = r;
    player_1p_name = p1_name;
    player_2p_name = p2_name;

    for (int i = 0; i < Game::TUMO_MAX; i++) {
        tumo_history.push_back(tumo[i]);
    }
}

void BattleHistory::load(std::string filename) {
    std::ifstream ifs(filename);

    if (ifs.fail()) {
        return;
    }

    // ƒcƒ‚‚ğ“Ç‚İ‚Ş
    std::string line;

    if (!std::getline(ifs, line)) {
        return;
    }

    std::stringstream tumo_ss(line);

    while (!tumo_ss.eof()) {
        int p, c;
        tumo_ss >> p >> c;
        tumo_history.push_back(Tumo(3, 12, Color(p), Color(c), 0));
    }

    // 1p‚Ì‘€ì‚ğ“Ç‚İ‚Ş
    if (!std::getline(ifs, line)) {
        return;
    }

    std::stringstream move_1p_ss(line);

    while (!tumo_ss.eof()) {
        int ob;
        move_1p_ss >> ob;
        move_history_1p.push_back(OperationBit(ob));
    }

    // 2p‚Ì‘€ì‚ğ“Ç‚İ‚Ş
    if (!std::getline(ifs, line)) {
        return;
    }

    std::stringstream move_2p_ss(line);

    while (!tumo_ss.eof()) {
        int ob;
        move_2p_ss >> ob;
        move_history_2p.push_back(OperationBit(ob));
    }
}

void BattleHistory::save(std::string filename) {
    std::ofstream ofs(filename);

    if (ofs.fail()) {
        return;
    }

    for (auto a : tumo_history) {
        ofs << a.color[0] << " " << a.color[1] << " ";
    }

    ofs << std::endl;

    for (auto a : move_history_1p) {
        ofs << a << " ";
    }

    ofs << std::endl;

    for (auto a : move_history_2p) {
        ofs << a << " ";
    }

    ofs.close();
}
