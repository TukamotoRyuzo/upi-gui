#pragma once

#include "pipe.h"
#include "field.h"
#include "move.h"
#include "engine.h"

#include <thread>

class UPIManager {
    static const int TUMO_MAX = 128;
    std::string getTumo128ToString(const Tumo* tumo) const;
public:
    // server -> client
    void upi(PipeManager& p);    
    void tumo(PipeManager& p, const Tumo* tumo);
    void position(PipeManager& p, Field& self, Field& enemy);
    void isready(PipeManager& p);
    void go(PipeManager& p);
    void quit(PipeManager& p);
    void gameover(PipeManager& p, bool win);

    // client -> server
    EngineInfo id(PipeManager& p);
    Move bestmove(PipeManager& p, Field& field);

    // set
    void setEngineMove(PipeManager& p, Field& self, Field& enemy, OperationQueue& queue);
    void launchEngine(PipeManager& p, const Tumo* tumo);
};