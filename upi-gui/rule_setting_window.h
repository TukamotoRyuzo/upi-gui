#pragma once

#include "window_base.h"
#include "rule.h"

// İ’è -> ƒ‹[ƒ‹İ’è‚ğ‰Ÿ‚µ‚½‚Æ‚«‚Éo‚Ä‚­‚é‰æ–Ê
class RuleSettingWindow : public WindowBase {
public:
    RuleSettingWindow(HINSTANCE hInst, Rule* rule);
protected:
    bool createWindow() override;
    bool onCreate() override;
    void setHandler() override;
    Rule* rule;
};

