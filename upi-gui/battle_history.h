#pragma once

#include "move.h"
#include "field.h"
#include <deque>

struct BattleHistory {
    // プレイヤー名
    std::string player_1p_name, player_2p_name;

    // この試合で採用されたルール
    Rule rule;

    // この試合で使用されたツモ
    std::vector<Tumo> tumo_history;

    // 操作の履歴
    std::deque<OperationBit> move_history_1p;
    std::deque<OperationBit> move_history_2p;

    void clear();
    void init(Rule r, std::string p1_name, std::string p2_name, const Tumo* tumo);
    bool load(std::string filename);
    void save(std::string filename);
};
