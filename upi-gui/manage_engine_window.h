#pragma once

#include "window_base.h"
#include "engine.h"

// İ’è -> ƒGƒ“ƒWƒ“ŠÇ—‚ğ‰Ÿ‚µ‚½‚Æ‚«‚Éo‚Ä‚­‚é‰æ–Ê
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
