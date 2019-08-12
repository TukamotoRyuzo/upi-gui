#pragma once

#include "window_base.h"
#include "game.h"
#include "engine.h"
#include "degug_window.h"
#include "rule_setting_window.h"
#include "sound.h"

enum FieldSize {
    // PLAYER1のフィールドの左上のx,y座標
    P1_F_BEGINX = 48,
    P1_F_BEGINY = 48,

    // PLAYER1のNEXTの左上のx,y座標
    P1_N_BEGINX = 408,
    P1_N_BEGINY = 70,

    // PLAYER1のNEXTNEXTの左上のx,y座標
    P1_NN_BEGINX = 461,
    P1_NN_BEGINY = 136,

    // PLAYER2のフィールドの左上のx,y座標
    P2_F_BEGINX = 641,
    P2_F_BEGINY = 48,

    // PLAYER2のNEXTの左上のx,y座標
    P2_N_BEGINX = 524,
    P2_N_BEGINY = 70,

    // PLAYER2のNEXTNEXTの左上のx,y座標
    P2_NN_BEGINX = 491,
    P2_NN_BEGINY = 136,

    // ぷよのいっぺんの長さ
    P_SIZE = 48,

    // ウィンドウサイズ
    WIN_WIDTH = 976,
    WIN_HEIGHT = 672,
};

// メインのウィンドウ
class MainWindow : public WindowBase {
public:
    MainWindow(HINSTANCE hInst);    
    HDC dcHandle(BitmapID bid) const;
    void show(BitmapID bid) const;
    void playVoice(int i, int j) const;
    void gameOver();
    void drawString(RECT* rect, TCHAR* str, UINT format);
    void rectClear(RECT* rect);
protected:
    bool initSound();
    bool initDC();
    bool initEngineList();
    bool createWindow() override;
    void onDestroy() override;
    bool onCreate() override;
    void onPaint() override;
    void onTimer() override;
    void setHandler() override;
    LRESULT onCtlColorStatic(WPARAM, LPARAM) override;

    struct BitmapMDC {
        LPSTR bitmap_name;
        HDC handle;
        HBITMAP prev;
        BitmapMDC() {};
        BitmapMDC(LPSTR c) : bitmap_name(c) {};
    };
    
    std::unordered_map<BitmapID, BitmapMDC> mdc;

    Sound ren_voice[2][11];
    UPIEngineManager engine_manager;
    Game game;
    DebugWindow* debug_window = nullptr;
};