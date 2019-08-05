#pragma once

#include "window_base.h"
#include "engine.h"

// �ݒ� -> �G���W���Ǘ����������Ƃ��ɏo�Ă�����
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
