#pragma once

#include <sstream>
#include <iomanip>
#include <deque>
#include <vector>
#include "common.h"

class Field;

enum Rank {
    RANK_0, RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_9, RANK_A, RANK_B, RANK_C, RANK_D, RANK_E, RANK_F, RANK_MAX
};

enum File {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_MAX
};

enum Square {
    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, Aa, Ab, Ac, Ad, Ae, Af,
    B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, Ba, Bb, Bc, Bd, Be, Bf,
    C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, Ca, Cb, Cc, Cd, Ce, Cf,
    D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, Da, Db, Dc, Dd, De, Df,
    E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, Ea, Eb, Ec, Ed, Ee, Ef,
    F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, Fa, Fb, Fc, Fd, Fe, Ff,
    SQ_MAX, DEADPOINT = Cc,
    SQ_ZERO = A0,
    SQ_UP = 1,
};

ENABLE_OPERATORS_ON(Rank);
ENABLE_OPERATORS_ON(File);
ENABLE_OPERATORS_ON(Square);
inline Square toSquare(int x, int y) { return Square(x * RANK_MAX + y); }
inline int rankOf(Square sq) { return sq % RANK_MAX; }
inline int fileOf(Square sq) { return sq / RANK_MAX; }

enum Move : uint16_t { MOVE_NONE, };
inline Move makeMove(const Square psq, const Square csq, const bool tigiri) { return Move(tigiri << 15 | csq << 8 | psq); }
inline Move makeMove(const Square psq, const Square csq) { return Move(csq << 8 | psq); }
inline Square psq(Move m) { return Square(m & 0x007f); }
inline Square csq(Move m) { return Square((m & 0x7f00) >> 8); }
inline bool isTigiri(Move m) { return (m & 0x8000) ? true : false; }

inline std::string toUPI(Move m) {
    std::stringstream ss;
    std::string rank_string = "abcdefghijklm";
    ss << fileOf(psq(m)) + 1 << rank_string[rankOf(psq(m))]
       << fileOf(csq(m)) + 1 << rank_string[rankOf(csq(m))];
    return ss.str();
}

struct MoveList : public std::vector<Move> {
    MoveList(const Field& f);
};

enum OperationBit {
    NO_OPERATION = 0,
    OPE_LEFT = 1 << 0,
    OPE_RIGHT = 1 << 1,
    OPE_DOWN = 1 << 2,
    OPE_R_ROTATE = 1 << 3,
    OPE_L_ROTATE = 1 << 4,
};

ENABLE_OPERATORS_ON(OperationBit);

struct OperationQueue {
    OperationBit pop() {
        OperationBit ret = queue.front();
        queue.pop_front();
        return ret;
    }

    void moveToOperation(Move m, const Field& f);
    void moveToOperationSoft(Move m, const Field& f);
    size_t size() const { return queue.size(); }
    void clear() { queue.clear(); }
private:
    std::deque<OperationBit> queue;
};

Move upiToMove(std::string upi_move, const Field& f);