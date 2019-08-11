#pragma once

#include <string>

struct Rule {
    int chain_time = 60;
    int next_time = 7;
    int set_time = 15;
    int fall_time = 2;
    int autodrop_time = 50;
    bool continuous = false;
    bool voice = false;
    bool ai_1p = false;
    bool ai_2p = false;
    std::string engine_name_1p = "";
    std::string engine_name_2p = "";
    void load();
    void save();
};