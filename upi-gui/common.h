#pragma once

#include <chrono>
#include <string>
#include <fstream>
#define NOMINMAX
#include <windows.h>
#include <cassert>

typedef   signed __int8    int8_t;
typedef unsigned __int8   uint8_t;
typedef   signed __int16  int16_t;
typedef unsigned __int16 uint16_t;
typedef   signed __int32  int32_t;
typedef unsigned __int32 uint32_t;
typedef   signed __int64  int64_t;
typedef unsigned __int64 uint64_t;

// 環境の差を考慮するため、これらは組み込み命令を使わず、ソフトウェア実装する。
// 64bitのうち、LSBの位置を返す。
inline int bsf64(const uint64_t mask) {
    static const int table[64] = {
        63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
        51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
        26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
        58, 20, 37, 17, 36, 8
    };

    const uint64_t tmp = mask ^ (mask - 1);
    const uint32_t old = static_cast<uint32_t>((tmp & 0xffffffff) ^ (tmp >> 32));
    return table[(old * 0x783a9b23) >> 26];
}

// 64bitのうち、たっているビットの数を返す。(popcntの代わり)
inline int popCount(uint64_t v) {
    uint64_t count = (v & 0x5555555555555555ULL) + ((v >> 1) & 0x5555555555555555ULL);
    count = (count & 0x3333333333333333ULL) + ((count >> 2) & 0x3333333333333333ULL);
    count = (count & 0x0f0f0f0f0f0f0f0fULL) + ((count >> 4) & 0x0f0f0f0f0f0f0f0fULL);
    count = (count & 0x00ff00ff00ff00ffULL) + ((count >> 8) & 0x00ff00ff00ff00ffULL);
    count = (count & 0x0000ffff0000ffffULL) + ((count >> 16) & 0x0000ffff0000ffffULL);
    return (int)((count & 0x00000000ffffffffULL) + ((count >> 32) & 0x00000000ffffffffULL));
}

typedef std::chrono::milliseconds::rep TimePoint;

// ms単位で現在時刻を返す
inline TimePoint now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ほとんど整数型と変わらないenum型に対して普通の算術演算、ビット演算ができるようにするマクロ
#define ENABLE_SAFE_OPERATORS_ON(T)\
    inline constexpr T  operator + (const T d1, const T d2) { return T(int(d1) + int(d2)); }\
    inline constexpr T  operator + (const T d1, const int d2) { return T(int(d1) + int(d2)); }\
    inline constexpr T  operator | (const T d1, const T d2) { return T(int(d1) | int(d2)); }\
    inline constexpr T  operator ^ (const T d1, const T d2) { return T(int(d1) ^ int(d2)); }\
    inline constexpr T  operator - (const T d1, const T d2) { return T(int(d1) - int(d2)); }\
    inline constexpr T  operator - (const T d1, const int d2) { return T(int(d1) - int(d2)); }\
    inline constexpr T  operator * (int i, const T d) { return T(i * int(d)); }\
    inline constexpr T  operator * (const T d, const T d2) { return T(int(d) * int(d2)); }\
    inline constexpr T  operator & (int i, const T d) { return T(i & int(d)); }\
    inline constexpr T  operator & (const T d1, const T d2) { return T(int(d1) & int(d2)); }\
    inline constexpr T  operator * (const T d, int i) { return T(int(d) * i); }\
    inline constexpr T  operator - (const T d) { return T(-int(d)); }\
    inline T& operator |= (T& d1, const T d2) { return d1 = d1 | d2; }\
    inline T& operator &= (T& d1, const T d2) { return d1 = d1 & d2; }\
    inline T& operator += (T& d1, const T d2) { return d1 = d1 + d2; }\
    inline T& operator -= (T& d1, const T d2) { return d1 = d1 - d2; }\
    inline T& operator += (T& d1, const int d2) { return d1 = d1 + d2; }\
    inline T& operator -= (T& d1, const int d2) { return d1 = d1 - d2; }\
    inline T& operator *= (T& d, int i) { return d = T(int(d) * i); }\
    inline T& operator ^= (T& d, int i) { return d = T(int(d) ^ i); }\
    inline constexpr T operator ~ (const T d) { return T(~int(d)); }

#define ENABLE_OPERATORS_ON(T) ENABLE_SAFE_OPERATORS_ON(T)\
    inline T& operator ++ (T& d) { return d = T(int(d) + 1); }\
    inline T& operator -- (T& d) { return d = T(int(d) - 1); }\
    inline T  operator ++ (T& d,int) { T prev = d; d = T(int(d) + 1); return prev; } \
    inline T  operator -- (T& d,int) { T prev = d; d = T(int(d) - 1); return prev; } \
    inline constexpr T  operator / (const T d, int i) { return T(int(d) / i); }\
    inline T& operator /= (T& d, int i) { return d = T(int(d) / i); }

inline void deleteCRLF(std::string& targetStr) {
    const char CR = '\r';
    const char LF = '\n';
    std::string destStr;
    for (const auto c : targetStr) {
        if (c != CR && c != LF && (destStr.size() != 0 || c != ' ')) {
            destStr += c;
        }
    }
    targetStr = std::move(destStr);
}

namespace Log {
    void start();
    void write(std::string str);
    void deleteLog();
    void end();
    void setDebugWindow(HWND hwnd);
}