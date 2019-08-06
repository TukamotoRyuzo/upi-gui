#include "main_window.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
    Log::start();    
    MainWindow main_window(hInst);

    if (!main_window.init()) {
        return -1;
    }
    
    // コモンコントロールを使えるようにする
    // http://wisdom.sakura.ne.jp/system/winapi/common/common8.html
    INITCOMMONCONTROLSEX ic;
    ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ic.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&ic);

    main_window.doMessageLoop();
    Log::end();
    return 0;
}
