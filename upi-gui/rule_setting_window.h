#pragma once

#include "window_base.h"
#include "rule.h"

// �ݒ� -> ���[���ݒ���������Ƃ��ɏo�Ă�����
class RuleSettingWindow : public WindowBase {
public:
    RuleSettingWindow(HINSTANCE hInst, Rule* rule);
protected:
    bool createWindow() override;
    bool onCreate() override;
    void setHandler() override;
    Rule* rule;
};

