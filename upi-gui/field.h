#pragma once

#include <cassert>
#include <random>
#include "common.h"

enum Time {
    CHAINTIME  = 60,
    NEXTTIME   = 7,
    SETTIME    = 15,
	FALLTIME   = 2,
	AUTODROP_FREQ = 50,
};

enum Color { 
	EMPTY, WALL, RED, GREEN, BLUE, PURPLE, YELLOW, OBSTACLE
};

inline Color randomColor(Color except_color = EMPTY) {
    std::random_device random;
    Color c;
    do { c = Color(random() % 5 + 2); } while (c == except_color);
    return c;
}

inline char colorToString(Color c) {
	return "  rgbpyo"[c];		
}

class Field;
class MainWindow;
enum OperationBit;
const int posi[4][2] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

struct Tumo {
    int x, y, rotate;
    Color color[2];
    Tumo(){};
    Tumo(int a, int b, Color col0, Color col1, int rot) :
        x(a), y(b), rotate(rot) {
        color[0] = col0;
        color[1] = col1;
    };

    int cx() const { return (x + posi[rotate][0]); }
    int cy() const { return (y + posi[rotate][1]); }
};

enum GameFase {
    NEXT, OPERATION, CHECK_SLIDE, CHECK_VANISH, CHAIN_VOICE,
    CHAIN, OJAMA_WAIT, OJAMA_FALLING,
    GAMEOVER,
};

enum PlayerStatus {
	NO_PLAYER_FLAG = 0,
	PLAYER1 = 1 << 0,
	PLAYER_AI = 1 << 2,
};

ENABLE_OPERATORS_ON(PlayerStatus);

class Field {
    // フィールドの横幅、高さ（番兵含める）
    static const int FIELD_WIDTH = 8;
    static const int FIELD_HEIGHT = 16;
	
    // おじゃま一個分の点数
    static const int RATE = 70;

    // 連鎖数
    int chain;

    // 現在の点数（おじゃまぷよが振るたびにリセットする）
    int score;

    // 点数の合計(表示用）
    int scoresum;

    // 現在、フィールド上部に表示される予定のお邪魔
    // [0] : setすると降るお邪魔
    // [1] : setしても降らないお邪魔
    int ojama[2];
    
    // 次にSETしたとき降るおじゃまぷよ
    int ojamabuf;

    // 現在のゲームのフェーズ
    GameFase fase;

    Tumo* tumo_pool;
	
	// タイマと、現在のぷよ配列番号
    int operation_timer, tumo_number;

    Color field[FIELD_WIDTH][FIELD_HEIGHT];

    // フェーズ間の待ち時間
    int wait_timer;

    // 現在操作中のツモ
    Tumo current;

    // 全消しフラグ
    bool all_clear;

    // 操作に関する制御用変数
    int Rcnt, Lcnt, Rrocnt, Lrocnt, Dcnt;

public:

    // 初期化
    Field(Tumo* t) : tumo_pool(t) {};

    void init();

    // ぷよを移動させるための関数群
    bool drop();

    // 連鎖後のぷよを落とす
    bool slide();

	// スライドに何フレームかかるかを返す。
	int slideFrame() const;

    // ぷよを消すための関数群
    void deletePuyo(int dammy[FIELD_WIDTH][FIELD_HEIGHT], int x, int y);
    bool vanish();
    int countConnection(int dammy[FIELD_WIDTH][FIELD_HEIGHT], int x, int y) const;

    // おじゃまぷよがふる
    void putOjama();

    // 表示
    void show(MainWindow* winapi_helper, PlayerStatus status);

    // ゲーム管理関数
    void update(Field &enemy, OperationBit key_operation);
    bool operateTumo(OperationBit key_operation);
    int convertSimulatorFormat(int score);
    void set(const Tumo &p);
    bool isEmpty(const Tumo &p) const;
    bool isEmpty();
    bool isEmpty(int x, int y) const { return field[x][y] == EMPTY; }
    bool isDeath() const { return (field[3][12] != EMPTY); }
    Color color(int x, int y) const { return field[x][y]; }
    int next(int x, int y, int d) const { return field[x + posi[d][0]][y + posi[d][1]]; }
    int con(int x, int y) const {
        int c = 0;
		for (int i = 0; i < 4; i++) {
			if (field[x][y] == next(x, y, i) && y + posi[i][1] < 13) {
				c |= 1 << i;
			}
		}
        return c;
    }

    void reload() {
        tumo_number = (tumo_number + 1) & 127;
        current = tumo_pool[tumo_number];
    }

    void unload() {
        tumo_number = (tumo_number - 1) & 127;
        current = tumo_pool[tumo_number];
    }

	Tumo getTumo() const {
		return tumo_pool[tumo_number];
	}

	void floorList(int* list) const {
		for (int i = 0; i < 6; i++) {
			int y;
			for (y = 13; y > 0; y--) {
				if (!isEmpty(i + 1, y)) {
					break;
				}
			}

			list[i] = y;
		}
	}

    bool operator == (const Field &f) const {
		for (int x = 0; x <= 7; x++) {
			for (int y = 0; y <= 15; y++) {
				if (field[x][y] != f.field[x][y]) {
					return false;
				}
			}
		}

        return true;
    }

	int getChain() const { return chain; }
	int getScoreSum() const { return scoresum; }
	GameFase getGamefase() const { return fase; }
	int getConformedOjama() const { return ojama[0]; }
	int getUnconformedOjama() const { return ojama[1]; }
	std::string toPfen() const;
};