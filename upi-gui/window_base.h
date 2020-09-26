#pragma once

#include "common.h"
#include <unordered_map>
#include <functional>
#include <MMSystem.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "winmm.lib")

enum EventID {
    B_START, B_STOP, B_RESTART,
    B_ADD_ENGINE, B_DELETE_ENGINE,
    B_SEND_MESSAGE_TO_ENGINE1, B_SEND_MESSAGE_TO_ENGINE2, B_DELETE_LOG,
    STATIC_FALL_FREQ,
    STATIC_CHAIN_WAIT_FRAME,
    STATIC_SET_FRAME,
    STATIC_NEXT_FRAME,
    STATIC_AUTODROP_FRAME,
    UPDOWN_FALL_FREQ,
    UPDOWN_CHAIN_WAIT_FRAME,
    UPDOWN_SET_FRAME,
    UPDOWN_NEXT_FRAME,
    UPDOWN_AUTODROP_FRAME,
    EDIT_DEBUG_LOG, EDIT_SEND_COMMAND,
    EDIT_FALL_FREQ,
    EDIT_CHAIN_WAIT_FRAME,
    EDIT_SET_FRAME,
    EDIT_NEXT_FRAME,
    EDIT_AUTODROP_FRAME,
    B_SAVE_PARAMS, B_RETURN_DEFAULT_VALUE,
    COMBO_AI1P, COMBO_AI2P,
    CHECK_AI1P, CHECK_AI2P,
    CHECK_CONTINUE_BATTLE, CHECK_PLAY_SOUND,
    LIST_ENGINE,
    MOK_START = 40001,
    MENU_OPEN_PUYOFU = MOK_START,
    MENU_SAVE_PUYOFU = 40002,
    MENU_END = 40003,
    MENU_FIELD_EDIT = 40004,
    MENU_BATTLE = 40005,
    MENU_SERVER_BATTLE = 40006,
    MENU_ADD_ENGINE = 40007,
    MENU_DEBUG_LOG = 40008,
    MENU_VERSION = 40009,
    MENU_RULE_SETTING = 40010,
    DEBUG_WINDOW_DESTROYED,
    MOK_END,
};

enum BitmapID {
    MEMORY,
    PUYO, FIELD,
    BITMAP_NB,
};

// WindowsAPIはこのクラスでラップして使う。
class WindowBase {
public:
    WindowBase(HINSTANCE hinst, LPSTR class_name);
    ~WindowBase();
    void doMessageLoop();
    bool init();
    HWND mainWindowHandle() const { return main_window_handle; }
protected:
    void create(EventID event_id);
    void createUpDown(EventID event_id, EventID bady, int upper, int lower, int now);
    void close(EventID event_id);
    void enable(EventID event_id);
    void disable(EventID event_id);
    void check(EventID event_id);
    void uncheck(EventID event_id);
    bool isChecked(EventID event_id) const;
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual bool createWindow() = 0;
    virtual bool onCreate() = 0;
    virtual void onDestroy();
    virtual void onPaint();
    virtual void onTimer() {};
    virtual void setHandler() {};
    virtual LRESULT onCtlColorStatic(WPARAM, LPARAM) { return (LRESULT)0; };
    std::function<void(HWND, UINT, WPARAM, LPARAM)>& commandHandler(EventID event_id);
    std::function<void(HWND, UINT, WPARAM, LPARAM)> emptyHandler();

    HINSTANCE instance_handle;
    HWND main_window_handle;

    struct ChildWindow {
        LPCSTR class_name, window_name;
        DWORD style;
        int x, y, w, h;
        HWND handle;
        std::function<void(HWND, UINT, WPARAM, LPARAM)> command_handler;
        ChildWindow() {};
        ChildWindow(LPCSTR c, LPCSTR w, DWORD s, int x_, int y_, int w_, int h_)
            : class_name(c), window_name(w), style(s), x(x_), y(y_), w(w_), h(h_) {};
    };

    std::unordered_map<EventID, ChildWindow> child_window;
    LPCSTR class_name;
};


#define VK_0 0x0030
#define VK_1 0x0031
#define VK_2 0x0032
#define VK_3 0x0033
#define VK_4 0x0034
#define VK_5 0x0035
#define VK_6 0x0036
#define VK_7 0x0037
#define VK_8 0x0038
#define VK_9 0x0039
#define VK_A 0x0041
#define VK_B 0x0042
#define VK_C 0x0043
#define VK_D 0x0044
#define VK_E 0x0045
#define VK_F 0x0046
#define VK_G 0x0047
#define VK_H 0x0048
#define VK_I 0x0049
#define VK_J 0x004A
#define VK_K 0x004B
#define VK_L 0x004C
#define VK_M 0x004D
#define VK_N 0x004E
#define VK_O 0x004F
#define VK_P 0x0050
#define VK_Q 0x0051
#define VK_R 0x0052
#define VK_S 0x0053
#define VK_T 0x0054
#define VK_U 0x0055
#define VK_V 0x0056
#define VK_W 0x0057
#define VK_X 0x0058
#define VK_Y 0x0059
#define VK_Z 0x005A
