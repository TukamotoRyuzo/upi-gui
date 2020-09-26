#pragma once

#include "window_base.h"
#include "pipe.h"

// �ݒ� -> �f�o�b�O���O���������Ƃ��ɏo�Ă�����
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
