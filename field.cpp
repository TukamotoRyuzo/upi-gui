#include <random>
#include <algorithm>

#include "field.h"
#include "common.h"
#include "main_window.h"
#include "move.h"

// field上にpuyoをセットする
void Field::set(const Tumo &p) {
    field[p.x][p.y] = p.color[0]; 
    field[p.cx()][p.cy()] = p.color[1]; 
}

bool Field::isEmpty(const Tumo &p) const {
    return isEmpty(p.x, p.y) && isEmpty(p.cx(), p.cy());
}

bool Field::isEmpty() {
	for (int x = 1; x <= 6; x++) {
		for (int y = 1; y <= 13; y++) {
			if (field[x][y] != EMPTY) {
				return false;
			}
		}
	}

    return true;
}

std::string Field::toPfen() const {
	std::stringstream ss;

	for (int x = 1; x <= 6; x++) {
		for (int y = 1; y <= 13; y++) {
			if (!isEmpty(x, y)) {
				ss << colorToString(color(x, y));
			}
			else {
				break;
			}
		}

		ss << "/";
	}

	ss << " " << tumo_number;
	return ss.str();
}

void Field::init() {
	this->all_clear = false;
	this->chain = 0;	
	this->fase = GameFase::NEXT;
	this->Dcnt = 0;
	this->Lcnt = 0;
	this->Lrocnt = 0;
	this->Rcnt = 0;
	this->Rrocnt = 0;
	this->tumo_number = -1;
	this->ojama[0] = 0;
	this->ojama[1] = 0;
	this->ojamabuf = 0;
	this->operation_timer = 0;
	this->score = 0;
	this->scoresum = 0;
	this->wait_timer = 0;    

	for (int x = 0; x < FIELD_WIDTH; x++) {
		for (int y = 0; y < FIELD_HEIGHT; y++) {
			field[x][y] = (x == 0 || x == 7 || y == 0 || y == 15) ? WALL : EMPTY;
		}
	}
}

// 一定時間ごとに一つ降りてくる処理
bool Field::drop() {
    // 一つ位置を下へ
    current.y--;

	if (isEmpty(current)) {
		return true;
	}

    // もし置けないなら、そこで位置を確定させる
    current.y++;
    set(current);
    return false;
}

// 下に一段ずらす。slideできなかったらfalseを返す
bool Field::slide() {
    bool ret = false;

	for (int x = 1; x <= 6; x++) {
		for (int y = 1; y <= 13; y++) {
			if (isEmpty(x, y)) {
				while (++y <= 13 + 1) {
					if (!isEmpty(x, y)) {
						ret = true;
					}

					field[x][y - 1] = field[x][y];
					field[x][y] = EMPTY;
				}
			}
		}
	}

    return ret;
}

// スライドに何フレームかかるかを返す。
int Field::slideFrame() const {
    bool ret = false;
    int slider_count[8] = { 0 };

	for (int x = 1; x <= 6; x++) {
		for (int y = 1; y <= 13; y++) {
			if (isEmpty(x, y)) {
				int dy = y;
				do {
					++dy;
				} while (isEmpty(x, dy));

				if (dy > 13) {
					break;
				}

				slider_count[x] += dy - y;
				y = dy;
			}
		}
	}

	return *std::max_element(slider_count, slider_count + 8);
}

bool Field::vanish() {
    // 連鎖ボーナス
    const int chain_b[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    const int connect_b[12] = { 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10 };
    const int color_b[5] = { 0, 3, 6, 12, 24 };

    int dammy[FIELD_WIDTH][FIELD_HEIGHT] = { 0 }, delete_count = 0, connect = 0, color_bit = 0, count;

	for (int x = 1; x <= 6; x++) {
		for (int y = 1; y <= 12; y++) {
			if (field[x][y] == EMPTY
				|| field[x][y] == OBSTACLE
				|| dammy[x][y] == 1) {
				continue;
			}

			else if ((count = countConnection(dammy, x, y)) >= 4) {
				delete_count += count;
				connect += connect_b[count % 8];
				color_bit |= 1 << color(x, y);
				deletePuyo(dammy, x, y);
			}
		}
	}

    if (delete_count >= 4) {
        int b = chain_b[chain] + connect + color_b[popCount(color_bit) - 1];
        b = std::min(std::max(b, 1), 999) * delete_count * 10;

        if (all_clear) {
            b += 30 * RATE;
            all_clear = false;
        }

        score += b;
        scoresum += b;
        chain++;
    }

	else {
		chain = 0;
	}

    return delete_count >= 4;
}

// フィールド上の同じ色を探し、つながっている個数を返す。
int Field::countConnection(int dammy[FIELD_WIDTH][FIELD_HEIGHT], int x, int y) const {
    int count = 1;
    dammy[x][y] = 1;

    for (int bp = con(x, y); bp; bp &= bp - 1) {
        int r = bsf64(bp);

		if (dammy[x + posi[r][0]][y + posi[r][1]] != 1) {
			count += countConnection(dammy, x + posi[r][0], y + posi[r][1]);
		}
    }

    return count;
}

void Field::deletePuyo(int dammy[FIELD_WIDTH][FIELD_HEIGHT], int x, int y) {
    dammy[x][y] = 2;

    for (int bp = con(x, y); bp; bp &= bp - 1) {
        int r = bsf64(bp);
		if (dammy[x + posi[r][0]][y + posi[r][1]] != 2) {
			deletePuyo(dammy, x + posi[r][0], y + posi[r][1]);
		}
    }

    field[x][y] = EMPTY;

    // おじゃまが回りにあったら、一緒に消す
	for (int r = 0; r < 4; r++) {
		if (field[x + posi[r][0]][y + posi[r][1]] == OBSTACLE) {
			field[x + posi[r][0]][y + posi[r][1]] = EMPTY;
		}
	}
}

// 14段目にお邪魔を並べる。その後はslideを呼べば良い。
void Field::putOjama() {
    if (ojamabuf) {
        // シャッフル
        int v[] = { 1, 2, 3, 4, 5, 6 };
        std::shuffle(v, std::end(v), std::mt19937());

        // 並べる個数は最大6個まで。
        int max = std::min(ojamabuf, 6);
        ojamabuf -= max;

		for (int x = 0; x < max; x++) {
			field[v[x]][14] = OBSTACLE;
		}
    }
}

// ↓が押された時trueが返る。
bool Field::operateTumo(OperationBit key_operation) {
    // 同時に押すことが許されない処理だけ、if-elseifにしている
    Tumo n = current;
    bool b = true;

    if (key_operation & OPE_LEFT) {
        if (Lcnt == 0 || Lcnt > 5 && Lcnt % 2 == 0) {
            n.x--;

			if (isEmpty(n)) {
				current = n;
			}
			else {
				n.x++;
			}
        }

        Lcnt++;
        Rcnt = 0;
    }

    else if (key_operation & OPE_RIGHT) {
        if (Rcnt == 0 || Rcnt > 5 && Rcnt % 2 == 0) {
            n.x++;

			if (isEmpty(n)) {
				current = n;
			}
			else {
				n.x--;
			}
        }

        Rcnt++;
        Lcnt = 0;
    }

	else if (Rrocnt < 5 && Lrocnt < 5) {
		Rcnt = Lcnt = 0;
	}

    if (key_operation & OPE_R_ROTATE) {
        if (Rrocnt == 0 || (Rrocnt > 999999 && Rcnt == 0 && Lcnt == 0 && Dcnt == 0)) {
            n.rotate = (n.rotate + 1) & 3;

            if (!isEmpty(n)) {
                // 回転させたぷよが横向き＝現在のぷよが縦向き
                if (n.rotate & 1) {
                    // 回転できなかったら左右どちらかが壁になっているので壁と反対側に移動。
                    n.x += n.rotate - 2;

                    // それでも置けなかったら、今度は左右が壁になっているので、2回転させる
                    if (!isEmpty(n)) {
                        // 元に戻す
                        n.x -= n.rotate - 2;

                        // 右上が開いている
                        if (isEmpty(n.x + 1, n.y + 1) && n.rotate == 1) {
                            n.y++;
                            n.rotate++;
                        }

                        else
                            n.rotate = (n.rotate + 1) & 3;

                        if (!isEmpty(n)) {
                            n.y++;

                            // ここまできて置けなかったら、15段目の壁にぶつかっているので操作しない
							if (!isEmpty(n)) {
								b = false;
							}
                        }
                    }
                }

                // 横向き
                else {
                    n.y++;

					if (!isEmpty(n)) {
						b = false;
					}
                }
            }

			if (b) {
				current = n;
			}
        }

        Rrocnt++;
    }

    else if (key_operation & OPE_L_ROTATE) {
        if (Lrocnt == 0 || (Lrocnt > 9999999 && Rcnt == 0 && Lcnt == 0 && Dcnt == 0)) {
            // 左回転
            n.rotate = (n.rotate + 3) & 3;

            if (!isEmpty(n)) {
                // 現在のぷよが縦向き
                if (n.rotate & 1) {
                    // 回転できなかったら左右どちらかが壁になっている
                    n.x += n.rotate - 2;

                    if (!isEmpty(n)) {
                        // それでも置けなかったら、今度は左右が壁になっているので、2回転させる
                        n.x -= n.rotate - 2;

                        // 左上が開いている
                        if (isEmpty(n.x - 1, n.y + 1) && n.rotate == 3) {
                            n.y++;
                            n.rotate--;
                        }

                        else
                            n.rotate = (n.rotate + 3) & 3;

                        if (!isEmpty(n)) {
                            n.y++;

                            // ここまできて置けなかったら、15段目の壁にぶつかっているので操作しない
							if (!isEmpty(n)) {
								b = false;
							}
                        }
                    }
                }

                else {
                    n.y++;

                    // ここまできて置けなかったら、15段目の壁にぶつかっているので操作しない
					if (!isEmpty(n)) {
						b = false;
					}
                }
            }

			if (b) {
				current = n;
			}
        }

        Lrocnt++;
    }

	else {
		Rrocnt = Lrocnt = 0;
	}

    bool down = false;

    if (key_operation & OPE_DOWN) {
        if (Dcnt++ % FALLTIME == 0) {
            // 下に移動するのではなく、一定時間ごとに下に下りていく処理を呼ぶためにtrueを返すようにしておく
            down = true;
        }
    }

	else if (Rrocnt < 5 && Lrocnt < 5) {
		Dcnt = 0;
	}

    return down;
}

// gameの処理
void Field::update(Field& enemy, OperationBit key_operation) {
    // 硬直時間がある場合は何もしない
    if (wait_timer) {
        wait_timer--;
		return;
    }

    switch (fase) {
        // ツモ出現待ち状態
    case NEXT:
        reload();
        fase = OPERATION;
        break;

        // ツモ操作可能状態
    case OPERATION:
        if (operateTumo(key_operation)) {
            // これによりdrop()が呼ばれる
            operation_timer = 0;

            // 落下ボーナス
            score++;
            scoresum++;
        }

        // 自由落下
        if (operation_timer++ % AUTODROP_FREQ == 0 && !drop()) {
            fase = CHECK_SLIDE;
			Rrocnt = Lrocnt = Dcnt = Rcnt = Lcnt = 0;
            wait_timer = !Field(*this).slide() ? 0 : SETTIME;
        }

        break;

    case CHECK_SLIDE:
		if (slide()) {
			wait_timer = 1;
		}
		else {
			wait_timer = SETTIME;
			fase = CHECK_VANISH;
		}

		break;

        // ツモ設置完了状態。連鎖やフィールドのスライドを行う。
    case CHECK_VANISH:
        if (vanish()) {
            ojama[0] -= score / RATE;
            score %= RATE;

            // 余剰分があればojama[1]を相殺
            if (ojama[0] < 0) {
                ojama[1] += ojama[0];
                ojama[0] = 0;

                // さらに余剰分があれば相手にあげる
                if (ojama[1] < 0) {
                    enemy.ojama[1] -= ojama[1];
                    ojama[1] = 0;
                }
            }

            fase = CHAIN_VOICE;
        }

        else {
            // 全消し
			if (isEmpty()) {
				all_clear = true;
			}

            // 連鎖が終わったので、相手の振らないお邪魔バッファを振るお邪魔バッファに移し替える。
            if (enemy.ojama[1]) {
                enemy.ojama[0] += enemy.ojama[1];
                enemy.ojama[1] = 0;
            }
                
            fase = OJAMA_WAIT;
        }

        break;

    case CHAIN_VOICE: {
		int slide_frame = slideFrame();
		fase = CHECK_SLIDE;
		wait_timer = (chain == 1 && slide_frame == 0) ? 0 : CHAINTIME - slide_frame * 2;
		break;
	}
    case OJAMA_WAIT:
        // 30個以上たまっている場合は30個降る。
        ojamabuf += std::min(30, ojama[0]);
        ojama[0] -= std::min(30, ojama[0]);
        fase = OJAMA_FALLING;
        wait_timer = NEXTTIME;
        break;

    case OJAMA_FALLING:
        putOjama();

        if (!slide()) {
            // 連鎖が終わっても、[3][12]にぷよがある
			if (isDeath()) {
				fase = GAMEOVER;
			}

            else {
                // 14だんめのぷよを消去する
				for (int x = 1; x <= 6; x++) {
					field[x][14] = EMPTY;
				}

                fase = NEXT;
            }
        }

        break;

    default:
        assert(false);
        break;
    }
}

int Field::convertSimulatorFormat(int score) {
    // 面白そうな連鎖ならぷよシミュレータで読み込める形のURLを生成する
    FILE *fp;
    errno_t error;
	if ((error = fopen_s(&fp, "rensa.txt", "a+")) != 0) {
		return 0;
	}

    fprintf(fp, "http://www.puyop.com/s/");

    // 0 空 
    // 1 赤
    // 2 緑
    // 3 青
    // 4 黄
    // 5 紫
    // 6 お邪魔
    const char sin[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int conv_col[8] = { 0, -1, 1, 2, 3, 4, 5, 6 };
    char buf[128] = "";
    int colBit = 0;
    int cnt = 0;
    bool b = false;

    // 左上から順番に
	for (int y = 13; y >= 1; y--) {
		for (int x = 1; x <= 6; x++) {
			if (isEmpty(x, y) && !b) {
				continue;
			}

			b = true;

			if (x % 2) {
				colBit = 0;
				colBit |= conv_col[color(x, y)] << 3;
			}

			else {
				colBit |= conv_col[color(x, y)];
			}

			if (x % 2 == 0) {
				buf[cnt++] = sin[colBit];
			}
		}
	}

    buf[cnt] = '\0';
    fprintf(fp, "%s", buf);
    //fprintf(fp, " %drensa %dscore\n", chainMax, score);
    fclose(fp);
    return 1;
}

/*BOOL BitBlt(
HDC hdcDest, // コピー先デバイスコンテキストのハンドル
int nXDest,  // コピー先長方形の左上隅の x 座標
int nYDest,  // コピー先長方形の左上隅の y 座標
int nWidth,  // コピー先長方形の幅
int nHeight, // コピー先長方形の高さ
HDC hdcSrc,  // コピー元デバイスコンテキストのハンドル
int nXSrc,   // コピー元長方形の左上隅の x 座標
int nYSrc,   // コピー元長方形の左上隅の y 座標
DWORD dwRop  // ラスタオペレーションコード
);*/

void Field::show(MainWindow* main_window, PlayerStatus status) {
	if (wait_timer) {
		return;
	}
	// つながり方によるぷよの表示の切り替え配列
	// 0000 0
	// 0001 上 2
	// 0010 右 3
	// 0011 上右 6 
	// 0100 下 4
	// 0101 上下 7 
	// 0110 右下 9 
	// 0111 上右下 14 
	// 1000 左 5 
	// 1001 左上 8
	// 1010 左右 10 
	// 1011 左右上 12 
	// 1100 左下 11 
	// 1101 左下上 15 
	// 1110 左下右 13 
	// 1111 左下右上 16

	static const int con_color[16] = { 0, 2, 3, 6, 4, 7, 9, 14, 5, 8, 10, 12, 11, 15, 13, 16 };
	Tumo next1 = tumo_pool[(tumo_number + 1) & 127];
	Tumo next2 = tumo_pool[(tumo_number + 2) & 127];
	const bool is1p = status & PLAYER1;
	const int PN_N1_BEGINX = is1p ? P1_N_BEGINX : P2_N_BEGINX;
	const int PN_N1_BEGINY = is1p ? P1_N_BEGINY : P2_N_BEGINY;
	const int PN_N2_BEGINX = is1p ? P1_NN_BEGINX : P2_NN_BEGINX;
	const int PN_N2_BEGINY = is1p ? P1_NN_BEGINY : P2_NN_BEGINY;
	const int PN_F_BEGINX = is1p ? P1_F_BEGINX : P2_F_BEGINX;
	const int PN_F_BEGINY = is1p ? P1_F_BEGINY : P2_F_BEGINY;

	for (int i = 0; i < 2; i++) {
		// NEXTぷよを表示
		BitBlt(main_window->dcHandle(MEMORY),
			PN_N1_BEGINX,
			PN_N1_BEGINY + P_SIZE * (1 - i),// 逆順に表示する
			P_SIZE,
			P_SIZE,
			main_window->dcHandle(PUYO),
			P_SIZE,
			next1.color[i] * P_SIZE,
			SRCCOPY);

		// NEXTNEXTぷよを表示
		BitBlt(main_window->dcHandle(MEMORY),
			PN_N2_BEGINX,
			PN_N2_BEGINY + P_SIZE * (1 - i),
			P_SIZE / 2,
			P_SIZE,// 半分だけ表示
			main_window->dcHandle(PUYO),
			P_SIZE + (is1p ? 0 : (P_SIZE / 2)),
			next2.color[i] * P_SIZE,
			SRCCOPY);
	}

	for (int x = 1; x <= 6; x++) {
		for (int y = 1; y <= 12; y++) {
			BitBlt(main_window->dcHandle(MEMORY),
				PN_F_BEGINX + (x - 1) * P_SIZE,
				PN_F_BEGINY + (12 - y) * P_SIZE,
				P_SIZE,
				P_SIZE,
				main_window->dcHandle(PUYO),
				(color(x, y) == OBSTACLE) ? 0 : P_SIZE * con_color[con(x, y)],
				(color(x, y)) * P_SIZE,
				SRCCOPY);
		}
	}

	// currentツモを表示
	if (fase == OPERATION) {
		for (int i = 0; i < 2; i++) {
			int x = i == 0 ? current.x : current.cx();
			int y = i == 0 ? current.y : current.cy();

			if (y < 13) {
				BitBlt(main_window->dcHandle(MEMORY),
					PN_F_BEGINX + (x - 1) * P_SIZE,
					PN_F_BEGINY + (12 - y) * P_SIZE,
					P_SIZE,
					P_SIZE,
					main_window->dcHandle(PUYO),
					0,
					current.color[i] * P_SIZE,
					SRCCOPY);
			}
		}
	}

	// おじゃまの表示
	// ちびぷよ = 1
	// ちびぷよ2 = 2
	// でかぷよ = 6
	// 岩 = 30
	// auaua = 200
	// 泣いてるやつ = 300
	// 黒いやつ = 400
	// 紫 = 500

	RECT rectP1ojama = { 48, 0, 336, 48 };
	RECT rectP2ojama = { 640, 0, 925, 48 };
	int ojama_kind[8] = { 1, 2, 6, 30, 200, 300, 400, 500 };
	int ojama_picture[8] = { 0 };
	int ojama_w = ojama[0] + ojama[1];

	RECT rect_ojama = is1p ? rectP1ojama : rectP2ojama;

	if (!ojama) {
		main_window->rectClear(&rect_ojama);
	}

	for (int i = 7; i >= 0; i--) {
		ojama_picture[i] = ojama_w / ojama_kind[i];
		ojama_w %= ojama_kind[i];
	}

	for (int i = 0; i < 6; i++) {
		int j;

		for (j = 8; j >= 1; j--) {
			if (ojama_picture[j - 1]) {
				ojama_picture[j - 1]--;
				break;
			}
		}

		const int dx = is1p ? 48 + P_SIZE * i : 640 + P_SIZE * i;
		const int sx = 816;
		BitBlt(main_window->dcHandle(MEMORY), dx, 0, P_SIZE, P_SIZE, main_window->dcHandle(PUYO), sx, P_SIZE * j, SRCCOPY);
	}

	const RECT rect_f1 = { P1_F_BEGINX, P_SIZE, P1_F_BEGINX + P_SIZE * 6, P_SIZE * 13 };
	const RECT rect_f2 = { P2_F_BEGINX, P_SIZE, P2_F_BEGINX + P_SIZE * 6, P_SIZE * 13 };
	const RECT rect_n1 = { 370, 50, 485, 232 };
	const RECT rect_n2 = { 490, 50, 605, 232 };
	InvalidateRect(main_window->mainWindowHandle(), is1p ? &rect_f1 : &rect_f2, false);
	InvalidateRect(main_window->mainWindowHandle(), is1p ? &rect_n1 : &rect_n2, false);
	InvalidateRect(main_window->mainWindowHandle(), &rect_ojama, false);
}