#pragma once

#include "window_base.h"
#include "pipe.h"

// 設定 -> デバッグログを押したときに出てくる画面
class DebugWindow : public WindowBase {
public:
    DebugWindow(HINSTANCE hInst, HWND hWnd, PipeManager* p1, PipeManager* p2);
protected:
    bool createWindow() override;
    void onDestroy() override;
    bool onCreate() override;
    void setHandler() override;
    HWND parent_window_handle;
    PipeManager* pipe1, * pipe2;
};
