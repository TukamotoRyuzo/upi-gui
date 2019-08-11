#pragma once

#include "battle_history.h"
#include "field.h"
#include "pipe.h"
#include "move.h"
#include "upi.h"

class MainWindow;

class Game {
public:
    static const int TUMO_MAX = 128;
private:
    struct Player {
        int max_chain, win, frame;
        Field field;
        PlayerStatus status;
        OperationQueue operation;
        PipeManager pipe;
        std::string name;
        Player(PlayerStatus ps, Tumo* tumo, Rule* rule) 
            : max_chain(0), win(0), frame(0), field(tumo, rule), status(ps)
        {};
        void init() {
            max_chain = 0;
            field.init();
            operation.clear();
        }
    };

    Tumo tumos[TUMO_MAX];
    TimePoint start_time, stop_time;
    UPIManager upi;
    MainWindow* main_window;
    
public:
    bool replay_mode;
    BattleHistory battle_history;
    Rule rule;
    Player p1, p2;
    Game(MainWindow* w) : p1(PLAYER1, tumos, &rule), p2(NO_PLAYER_FLAG, tumos, &rule), main_window(w) {
        rule.load();
    };
    void init();
    void initTumo();
    const Tumo* getTumo() const {
        return tumos;
    };
    void stop();
    void restart();
    void onFase(GameFase bf, GameFase gf, Player& player, int chain);
    void show();
    void update();
    OperationBit getKeyOperation(PlayerStatus status);
};
