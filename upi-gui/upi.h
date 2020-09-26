#pragma once

#include "pipe.h"
#include "field.h"
#include "move.h"
#include "engine.h"

#include <thread>
#include <atomic>

class UPIManager {
    static const int TUMO_MAX = 128;
    std::string getTumo128ToString(const Tumo* tumo) const;
    std::thread receive_thread;
    std::atomic_bool active;
public:
    PipeManager pipe;

    // server -> client
    void upi();
    void tumo(const Tumo* tumo);
    void rule(Rule& r);
    void position(Field& self, Field& enemy);
    void isready();
    void go();
    void quit();
    void gameover(bool win);
    void illegalmove(Move move);

    // client -> server
    EngineInfo id();
    Move bestmove(Field& field);

    // set
    bool setEngineMove(Field& self, Field& enemy, OperationQueue& queue);
    void launchEngine(const Tumo* t, Rule& r);

    // レシーブスレッドの開始
    void doRecvLoop();
};