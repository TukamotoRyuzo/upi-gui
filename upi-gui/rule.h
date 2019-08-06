#pragma once

struct Rule {
    int chain_time = 60;
    int next_time = 7;
    int set_time = 15;
    int fall_time = 2;
    int autodrop_time = 50;
    void load();
    void save();
};
