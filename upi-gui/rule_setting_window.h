#pragma once

#include "window_base.h"
#include "rule.h"

// 設定 -> ルール設定を押したときに出てくる画面
class RuleSettingWindow : public WindowBase {
public:
    RuleSettingWindow(HINSTANCE hInst, Rule* rule);
protected:
    bool createWindow() override;
    bool onCreate() override;
    void setHandler() override;
    Rule* rule;
};

