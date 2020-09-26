#include "rule_setting_window.h"

#include <algorithm>
#include <exception>
#include <stdexcept>

RuleSettingWindow::RuleSettingWindow(HINSTANCE hInst, Rule* r) :
    WindowBase(hInst, TEXT("ルール設定")),
    rule(r) {
    const DWORD style_edit = WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL;
    const DWORD style_button = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    const DWORD style_static = WS_CHILD | WS_VISIBLE | SS_LEFT;

    const int start_edit_x = 170;
    const int start_static_x = 20;
    const int height = 25;
    const int static_width = 140;

    child_window[STATIC_FALL_FREQ] = ChildWindow(TEXT("STATIC"), TEXT("落下速度"), style_static, start_static_x, 20, static_width, height);
    child_window[STATIC_CHAIN_WAIT_FRAME] = ChildWindow(TEXT("STATIC"), TEXT("連鎖時間"), style_static, start_static_x, 50, static_width, height);
    child_window[STATIC_SET_FRAME] = ChildWindow(TEXT("STATIC"), TEXT("設置時間"), style_static, start_static_x, 80, static_width, height);
    child_window[STATIC_NEXT_FRAME] = ChildWindow(TEXT("STATIC"), TEXT("ネクスト出現時間"), style_static, start_static_x, 110, static_width, height);
    child_window[STATIC_AUTODROP_FRAME] = ChildWindow(TEXT("STATIC"), TEXT("自由落下時間"), style_static, start_static_x, 140, static_width, height);

    child_window[EDIT_FALL_FREQ] = ChildWindow(TEXT("edit"), NULL, style_edit, start_edit_x, 20, 60, height);
    child_window[EDIT_CHAIN_WAIT_FRAME] = ChildWindow(TEXT("edit"), NULL, style_edit, start_edit_x, 50, 60, height);
    child_window[EDIT_SET_FRAME] = ChildWindow(TEXT("edit"), NULL, style_edit, start_edit_x, 80, 60, height);
    child_window[EDIT_NEXT_FRAME] = ChildWindow(TEXT("edit"), NULL, style_edit, start_edit_x, 110, 60, height);
    child_window[EDIT_AUTODROP_FRAME] = ChildWindow(TEXT("edit"), NULL, style_edit, start_edit_x, 140, 60, height);
    child_window[B_SAVE_PARAMS] = ChildWindow(TEXT("button"), TEXT("保存"), style_button, 410, 180, 60, height);
    child_window[B_RETURN_DEFAULT_VALUE] = ChildWindow(TEXT("button"), TEXT("デフォルト値に戻す"), style_button, 230, 180, 160, height);

    for (EventID e = UPDOWN_FALL_FREQ; e <= UPDOWN_AUTODROP_FRAME; e = EventID(e + 1)) {
        child_window[e] = ChildWindow();
    }
}

bool RuleSettingWindow::createWindow() {
    // windowクラス登録
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WindowBase::windowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance_handle;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = class_name;
    wc.hIconSm = NULL;

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    // windowサイズ設定
    RECT r = { 0, 0, 500, 230 };

    // ウィンドウサイズの自動計算
    AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION, false, 0);

    // ウィンドウ生成、表示
    main_window_handle = CreateWindow(class_name, class_name,
        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        NULL, NULL, instance_handle, NULL);

    return main_window_handle != NULL;
}

bool RuleSettingWindow::onCreate() {
    create(EDIT_FALL_FREQ);
    create(EDIT_CHAIN_WAIT_FRAME);
    create(EDIT_SET_FRAME);
    create(EDIT_NEXT_FRAME);
    create(EDIT_AUTODROP_FRAME);
    create(B_SAVE_PARAMS);
    create(B_RETURN_DEFAULT_VALUE);
    create(STATIC_FALL_FREQ);
    create(STATIC_CHAIN_WAIT_FRAME);
    create(STATIC_SET_FRAME);
    create(STATIC_NEXT_FRAME);
    create(STATIC_AUTODROP_FRAME);
    createUpDown(UPDOWN_FALL_FREQ, EDIT_FALL_FREQ, 1024, 1, rule->fall_time);
    createUpDown(UPDOWN_CHAIN_WAIT_FRAME, EDIT_CHAIN_WAIT_FRAME, 1024, 1, rule->chain_time);
    createUpDown(UPDOWN_SET_FRAME, EDIT_SET_FRAME, 1024, 1, rule->set_time);
    createUpDown(UPDOWN_NEXT_FRAME, EDIT_NEXT_FRAME, 1024, 1, rule->next_time);
    createUpDown(UPDOWN_AUTODROP_FRAME, EDIT_AUTODROP_FRAME, 1024, 1, rule->autodrop_time);

    // 文字数制限をかけておこう
    SendMessage(child_window.at(EDIT_FALL_FREQ).handle, EM_SETLIMITTEXT, (WPARAM)4, 0);
    SendMessage(child_window.at(EDIT_CHAIN_WAIT_FRAME).handle, EM_SETLIMITTEXT, (WPARAM)4, 0);
    SendMessage(child_window.at(EDIT_SET_FRAME).handle, EM_SETLIMITTEXT, (WPARAM)4, 0);
    SendMessage(child_window.at(EDIT_NEXT_FRAME).handle, EM_SETLIMITTEXT, (WPARAM)4, 0);
    SendMessage(child_window.at(EDIT_AUTODROP_FRAME).handle, EM_SETLIMITTEXT, (WPARAM)4, 0);

    ShowWindow(main_window_handle, SW_SHOW);
    return true;
}

void RuleSettingWindow::setHandler() {
    commandHandler(EDIT_FALL_FREQ) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    };
    commandHandler(EDIT_CHAIN_WAIT_FRAME) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    };
    commandHandler(EDIT_SET_FRAME) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    };
    commandHandler(EDIT_NEXT_FRAME) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    };
    commandHandler(EDIT_AUTODROP_FRAME) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    };
    commandHandler(B_SAVE_PARAMS) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

        auto get_int = [&](EventID eid) {
            char buf[1024];

            // エディットコントロールから通知を読み込む。
            GetWindowText(child_window.at(eid).handle, buf, 4);
            std::string str(buf);

            if (!std::all_of(str.cbegin(), str.cend(), isdigit)) {
                throw std::runtime_error("数値でない値が入っています。");
            }

            return std::stoi(str);
        };

        try {
            // 一度仮に読み込んでみる
            Rule r;
            r.fall_time = get_int(EDIT_FALL_FREQ);
            r.chain_time = get_int(EDIT_CHAIN_WAIT_FRAME);
            r.set_time = get_int(EDIT_SET_FRAME);
            r.next_time = get_int(EDIT_NEXT_FRAME);
            r.autodrop_time = get_int(EDIT_AUTODROP_FRAME);

            // エラーが発生しなければ、本チャンのRuleに読み込む。
            *rule = r;
            rule->save();
        }
        catch (std::exception& ex) {
            MessageBox(NULL, ex.what(), "error", MB_OK);
        }
    };

    commandHandler(B_RETURN_DEFAULT_VALUE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Rule r;
        SetWindowText(child_window.at(EDIT_FALL_FREQ).handle, std::to_string(r.fall_time).c_str());
        SetWindowText(child_window.at(EDIT_CHAIN_WAIT_FRAME).handle, std::to_string(r.chain_time).c_str());
        SetWindowText(child_window.at(EDIT_SET_FRAME).handle, std::to_string(r.set_time).c_str());
        SetWindowText(child_window.at(EDIT_NEXT_FRAME).handle, std::to_string(r.next_time).c_str());
        SetWindowText(child_window.at(EDIT_AUTODROP_FRAME).handle, std::to_string(r.autodrop_time).c_str());
    };
}
