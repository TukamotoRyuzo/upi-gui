#pragma once

#include "move.h"
#include "field.h"
#include <deque>

struct BattleHistory {
    // �v���C���[��
    std::string player_1p_name, player_2p_name;

    // ���̎����ō̗p���ꂽ���[��
    Rule rule;

    // ���̎����Ŏg�p���ꂽ�c��
    std::vector<Tumo> tumo_history;

    // ����̗���
    std::deque<OperationBit> move_history_1p;
    std::deque<OperationBit> move_history_2p;

    void clear();
    void init(Rule r, std::string p1_name, std::string p2_name, const Tumo* tumo);
    bool load(std::string filename);
    void save(std::string filename);
};
