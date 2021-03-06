#pragma once

#include "window_base.h"
#include "engine.h"

// 設定 -> エンジン管理を押したときに出てくる画面
class ManageEngineWindow : public WindowBase {
public:
    ManageEngineWindow(HINSTANCE hInst, UPIEngineManager* e);
protected:
    bool createWindow() override;
    bool onCreate() override;
    void setHandler() override;
    void initEngineList();
    UPIEngineManager* engine_manager;
};
