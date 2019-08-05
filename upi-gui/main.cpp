#include "main_window.h"
#include "game.h"
#include <iostream>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
    Log::start();    
    MainWindow main_window(hInst);

    if (!main_window.init()) {
        return -1;
    }

    main_window.doMessageLoop();
    Log::end();
    return 0;
}
