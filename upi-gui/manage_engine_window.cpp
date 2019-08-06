#include "manage_engine_window.h"

ManageEngineWindow::ManageEngineWindow(HINSTANCE hInst, UPIEngineManager* e) 
    : WindowBase(hInst, TEXT("エンジン管理")), engine_manager(e) {
    const DWORD style_button = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    const DWORD style_listbox = WS_CHILD | WS_VISIBLE | LBS_STANDARD;

    // button
    child_window[B_ADD_ENGINE] = ChildWindow(TEXT("BUTTON"), TEXT("追加"), style_button, 400, 30, 60, 30);
    child_window[B_DELETE_ENGINE] = ChildWindow(TEXT("BUTTON"), TEXT("削除"), style_button, 400, 70, 60, 30);

    // list
    child_window[LIST_ENGINE] = ChildWindow(TEXT("LISTBOX"), TEXT("エンジン一覧"), style_listbox, 30, 30, 350, 530);
}

bool ManageEngineWindow::createWindow() {
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
    RECT r = { 0, 0, 500, 600 };

    // ウィンドウサイズの自動計算
    AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION, false, 0);

    // ウィンドウ生成、表示
    main_window_handle = CreateWindow(class_name, class_name,
        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        NULL, NULL, instance_handle, NULL);

    return main_window_handle != NULL;
}

bool ManageEngineWindow::onCreate() {
    create(B_ADD_ENGINE);
    create(B_DELETE_ENGINE);
    create(LIST_ENGINE);
    disable(B_DELETE_ENGINE);

    initEngineList();
    InvalidateRect(main_window_handle, NULL, false);
    ShowWindow(main_window_handle, SW_SHOW);

    return true;
}

void ManageEngineWindow::setHandler() {
    commandHandler(B_ADD_ENGINE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        static OPENFILENAME ofn = { 0 };
        static TCHAR szPath[MAX_PATH] = ".";
        static TCHAR szFile[MAX_PATH];
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hWnd;
        ofn.lpstrInitialDir = NULL; // 初期フォルダ位置
        ofn.lpstrFile = szFile;       // 選択ファイル格納
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = TEXT("exeファイル(*.exe)\0*.exe;\0")
            TEXT("すべてのファイル(*.*)\0*.*\0");
        ofn.lpstrTitle = TEXT("思考エンジンを選択");

        // OFN_NOCHANGEDIR: カレントディレクトリを変更しないフラグ
        // これを指定しないとファイルを選択したときにカレントディレクトリが選択したファイルの場所に変更されてしまう。
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileName(&ofn)) {
            if (engine_manager->addUPIEngine(mainWindowHandle(), ofn.lpstrFile)) {
                initEngineList();
            }
        }
    };

    commandHandler(B_DELETE_ENGINE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // 現在選択中のエンジン名のindex取得
        auto index = SendMessage(child_window[LIST_ENGINE].handle, LB_GETCURSEL, 0, 0);

        // リストから削除
        SendMessage(child_window[LIST_ENGINE].handle, LB_DELETESTRING, index, 0);

        // engine.csvからも削除
        engine_manager->deleteEngine(index);

        // 削除ボタン非アクティブ
        disable(B_DELETE_ENGINE);
    };

    commandHandler(LIST_ENGINE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (HIWORD(wParam) == LBN_SELCHANGE) {
            enable(B_DELETE_ENGINE);
        }
    };
}

void ManageEngineWindow::initEngineList() {
    engine_manager->initEngineList();
    auto engine_list = engine_manager->getUPIEngineList();

    // 既に登録されているエンジン名があれば削除    
    SendMessage(child_window[LIST_ENGINE].handle, LB_RESETCONTENT, 0, 0);

    // リストボックスの初期化
    for (int i = 0; i < engine_list.size(); i++) {
        SendMessage(child_window[LIST_ENGINE].handle, LB_INSERTSTRING, i, (LPARAM)engine_list[i].engine_name.c_str());
    }
}
