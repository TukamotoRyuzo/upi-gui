#include "common.h"
#include <windows.h>
#include <sstream>

namespace Log {
    std::ofstream log;
    std::stringstream log_memory;

    // グローバル変数だけどまあいいだろう。
    HWND debug_edit_control = NULL;

    void Log::start() {
        log.open("upi.log");
    }

    void write(std::string str) {
        log << str;
        log.flush();
        log_memory << str;

        if (debug_edit_control != NULL) {
            SetWindowText(debug_edit_control, log_memory.str().c_str());
            SendMessage(debug_edit_control, WM_VSCROLL, SB_BOTTOM, NULL);
        }
    }

    void deleteLog() {
        log_memory.str("");
        log_memory.clear(std::stringstream::goodbit);

        if (debug_edit_control != NULL) {
            SetWindowText(debug_edit_control, log_memory.str().c_str());
        }
    }

    void Log::end() {
        log.close();
    }

    void setDebugWindow(HWND hwnd) {
        debug_edit_control = hwnd;
    }
}
