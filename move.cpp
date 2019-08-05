#include "move.h"
#include "field.h"

// 範囲を決める
void setRange(int* flist, int* left, int* right) {
	*left = 0;
	*right = 5;

	// 調べる範囲を決める
	for (int x = 1; x >= 0; x--) {
		if (flist[x] >= 12) {
			*left = x + 1;
			break;
		}
	}

	for (int x = 3; x <= 5; x++) {
		if (flist[x] >= 12) {
			*right = x - 1;
			break;
		}
	}
}

// 指して生成
MoveList::MoveList(const Field& f) {
	if (f.isDeath()) {
		return;
	}

	Tumo now = f.getTumo();
	int floor_list[6], right, left, x;
	f.floorList(floor_list);
	setRange(floor_list, &left, &right);

	if (now.color[0] == now.color[1]) {
		for (x = left; x < right; x++) {
			int up = floor_list[x];
			int up_side = floor_list[x + 1];
			Square dest = toSquare(x, up);
			Square dest_up = dest + SQ_UP;
			Square dest_side = toSquare(x + 1, up_side);
			bool is_tigiri = (up != up_side);
			push_back(makeMove(dest, dest_up));
			push_back(makeMove(dest, dest_side));
		}

		Square dest = toSquare(x, floor_list[x]);
		Square dest_up = dest + SQ_UP;
		push_back(makeMove(dest, dest_up));
	}
	else {
		for (x = left; x < right; x++) {
			int up = floor_list[x];
			int up_side = floor_list[x + 1];
			Square dest = toSquare(x, up);
			Square dest_up = dest + SQ_UP;
			Square dest_side = toSquare(x + 1, up_side);
			bool is_tigiri = (up != up_side);
			push_back(makeMove(dest, dest_up));
			push_back(makeMove(dest_up, dest));
			push_back(makeMove(dest, dest_side, is_tigiri));
			push_back(makeMove(dest_side, dest, is_tigiri));
		}

		Square dest = toSquare(x, floor_list[x]);
		Square dest_up = dest + SQ_UP;
		push_back(makeMove(dest, dest_up));
		push_back(makeMove(dest_up, dest));
	}
}

// upi形式の指し手を内部データに変換する。
Move upiToMove(std::string upi_move, const Field& f) {
	for (auto m : MoveList(f)) {
		if (toUPI(m) == upi_move) {
			return m;
		}
	}

	return MOVE_NONE;
}

// 指し手を受け取って操作に変換する。
// 頑張って最速の操作にしているが、頑張らなければもっと短くできる。
void OperationQueue::moveToOperation(Move m, const Field& f) {
	queue.clear();

	// 必要な横移動と下移動、回転数
	int x = fileOf(psq(m)) + 1;
	int y = rankOf(psq(m)) + 1;
	int cx = fileOf(csq(m)) + 1;
	int cy = rankOf(csq(m)) + 1;
	int r = 0;
	if (x == cx) {
		if (y == cy + 1) {
			r = 2;
		}
		else {
			r = 0;
		}
	}
	else {
		if (x == cx + 1) {
			r = 3;
		}
		else {
			r = 1;
		}
	}

	int n = 0;
	int necX = x - 3;
	int necY = 12 - (y > y + posi[r][1] ? y : y + posi[r][1]);
	int necR = r;

	if (necY < 0) {
		necY = 0;
	}

	for (int i = 0; i < 40; i++) {
		// まず↓ボタンは押しっぱなしにする
		queue.push_back(OPE_DOWN);

		if (necX > 0 && i % 2 == 0 && i < necX * 2) {
			queue[i] |= OPE_RIGHT;
		}

		else if (necX < 0 && i % 2 == 0 && i < abs(necX) * 2) {
			queue[i] |= OPE_LEFT;
		}
	}

	// 右移動
	if (necX > 0) {
		if (necX == 3) {
			if (!f.isEmpty(4, 8)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(4, 9)) {
					queue[1] &= ~OPE_DOWN;

					if (!f.isEmpty(4, 10)) {
						queue[2] &= ~OPE_DOWN;

						if (!f.isEmpty(4, 11)) {
							queue[3] &= ~OPE_DOWN;
						}
					}
				}
			}

			if (!f.isEmpty(5, 7)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(5, 8)) {
					queue[1] &= ~OPE_DOWN;
					queue[2] &= ~OPE_DOWN;

					if (!f.isEmpty(5, 9)) {
						queue[3] &= ~OPE_DOWN;
					}
				}
			}
		}

		else if (necX == 2) {
			if (!f.isEmpty(4, 9)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(4, 10)) {
					queue[1] &= ~OPE_DOWN;

					if (!f.isEmpty(4, 11)) {
						queue[2] &= ~OPE_DOWN;
					}
				}
			}

			if (!f.isEmpty(5, 9)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(5, 10)) {
					queue[1] &= ~OPE_DOWN;
					queue[2] &= ~OPE_DOWN;

					if (!f.isEmpty(5, 11)) {
						queue[3] &= ~OPE_DOWN;
					}
				}
			}
		}

		if (necR == 3) {
			if (!f.isEmpty(3, 10)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(3, 11)) {
					queue[1] &= ~OPE_DOWN;
				}
			}

			queue[0] |= OPE_L_ROTATE;
		}

		else if (necR == 2) {
			if (!f.isEmpty(3, 10)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(3, 11)) {
					queue[1] &= ~OPE_DOWN;
				}
			}

			if (necX == 3) {
				if (necY == 0) {
					queue[4] |= OPE_L_ROTATE;
					queue[6] |= OPE_L_ROTATE;
				}
				else {
					queue[0] |= OPE_L_ROTATE;
					queue[2] |= OPE_L_ROTATE;
				}
			}
			else {
				queue[0] |= OPE_L_ROTATE;
				queue[2] |= OPE_L_ROTATE;
			}
		}

		else if (necR == 1) {
			queue[0] |= OPE_R_ROTATE;
		}
	}

	// 左移動
	else if (necX < 0) {
		if (!f.isEmpty(2, 10)) {
			queue[0] &= ~OPE_DOWN;

			if (!f.isEmpty(2, 11)) {
				queue[1] &= ~OPE_DOWN;
			}
		}

		if (necR == 3) {
			queue[0] |= OPE_L_ROTATE;
		}

		// 左移動時は左回転のほうが安全
		else if (necR == 2) {
			if (!f.isEmpty(3, 10)) {
				queue[0] &= ~OPE_DOWN;
				queue[1] &= ~OPE_DOWN;
			}

			if (necX == -2) {
				queue[0] |= OPE_R_ROTATE;
				queue[2] |= OPE_R_ROTATE;
			}
			else {
				queue[0] |= OPE_R_ROTATE;
				queue[2] |= OPE_R_ROTATE;
			}
		}

		else if (necR == 1) {
			if (!f.isEmpty(3, 10)) {
				queue[0] &= ~OPE_DOWN;

				if (!f.isEmpty(3, 11)) {
					queue[1] &= ~OPE_DOWN;
				}
			}

			queue[0] |= OPE_R_ROTATE;
		}
	}

	// 3列目におく場合
	else {
		if (necR == 3) {
			queue[0] |= OPE_L_ROTATE;
		}

		else if (necR == 2) {
			// 左右が詰まっている場合は、2回転は1回転ですむ
			if (!f.isEmpty(4, 12) && !f.isEmpty(2, 12)) {
				queue[0] |= OPE_R_ROTATE;
			}

			else if (!f.isEmpty(4, 12) && f.isEmpty(2, 12)) {
				queue[0] |= OPE_L_ROTATE;
				queue[2] |= OPE_L_ROTATE;
				queue[0] &= ~OPE_DOWN;
				queue[1] &= ~OPE_DOWN;
			}
			else if (!f.isEmpty(2, 12) && f.isEmpty(4, 12)) {
				queue[0] |= OPE_R_ROTATE;
				queue[2] |= OPE_R_ROTATE;
				queue[0] &= ~OPE_DOWN;
				queue[1] &= ~OPE_DOWN;
			}
			else if (!f.isEmpty(4, 10) || !f.isEmpty(2, 10)) {
				queue[0] |= OPE_L_ROTATE;
				queue[2] |= OPE_L_ROTATE;
				queue[0] &= ~OPE_DOWN;
				queue[1] &= ~OPE_DOWN;
			}
			else {
				queue[0] |= OPE_L_ROTATE;
				queue[2] |= OPE_L_ROTATE;
			}
		}

		else if (necR == 1) {
			if (!f.isEmpty(3, 11)) {
				queue[0] &= ~OPE_DOWN;
				queue[1] &= ~OPE_DOWN;
			}

			queue[0] |= OPE_R_ROTATE;
		}
	}

	// ここまでで、OPE_DOWNがいくつあるか確かめる
	int downCount = 0;
	int i;

	for (i = 0; i < 40 && downCount < necY; i++) {
		if (queue[i] & OPE_DOWN) {
			downCount++;
		}
	}

	for (; i < 40; i++) {
		if ((i < 6 && necX == 3) || (i < 4 && (necX == 2 || necX == -2)) || i < 2 && necR == 2) {
			queue[i] &= ~OPE_DOWN;
		}
	}

	// 横移動
	while (necX) {
		if (necX < 0) {
			// 左方向の移動
			queue[n++] |= OPE_LEFT;
			n++;// 連続する移動は受け付けられないので
			necX++;
		}
		else {
			queue[n++] |= OPE_RIGHT;
			n++;
			necX--;
		}
	}
}
