#pragma once

#include "field.h"
#include "pipe.h"
#include "move.h"
#include "upi.h"

class MainWindow;

class Game {
	static const int TUMO_MAX = 128;

	struct Player {
		int max_chain, win, frame;
		Field field;
		PlayerStatus status;
		OperationQueue operation;
		Move move;
		PipeManager pipe;
		Player(PlayerStatus ps, Tumo* tumo) 
			: max_chain(0), win(0), frame(0), field(tumo), status(ps), move(MOVE_NONE)
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
	Player p1, p2;
	Game(MainWindow* w) : p1(PLAYER1, tumos), p2(NO_PLAYER_FLAG, tumos), main_window(w) {};
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