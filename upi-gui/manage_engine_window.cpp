#include "manage_engine_window.h"

ManageEngineWindow::ManageEngineWindow(HINSTANCE hInst, UPIEngineManager* e) 
    : WindowBase(hInst, TEXT("�G���W���Ǘ�")), engine_manager(e) {
    const DWORD style_button = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    const DWORD style_listbox = WS_CHILD | WS_VISIBLE | LBS_STANDARD;

    // button
    child_window[B_ADD_ENGINE] = ChildWindow(TEXT("BUTTON"), TEXT("�ǉ�"), style_button, 400, 30, 60, 30);
    child_window[B_DELETE_ENGINE] = ChildWindow(TEXT("BUTTON"), TEXT("�폜"), style_button, 400, 70, 60, 30);

    // list
    child_window[LIST_ENGINE] = ChildWindow(TEXT("LISTBOX"), TEXT("�G���W���ꗗ"), style_listbox, 30, 30, 350, 530);
}

bool ManageEngineWindow::createWindow() {
    // window�N���X�o�^
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

    // window�T�C�Y�ݒ�
    RECT r = { 0, 0, 500, 600 };

    // �E�B���h�E�T�C�Y�̎����v�Z
    AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION, false, 0);

    // �E�B���h�E�����A�\��
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
        ofn.lpstrInitialDir = NULL; // �����t�H���_�ʒu
        ofn.lpstrFile = szFile;       // �I���t�@�C���i�[
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = TEXT("exe�t�@�C��(*.exe)\0*.exe;\0")
            TEXT("���ׂẴt�@�C��(*.*)\0*.*\0");
        ofn.lpstrTitle = TEXT("�v�l�G���W����I��");

        // OFN_NOCHANGEDIR: �J�����g�f�B���N�g����ύX���Ȃ��t���O
        // ������w�肵�Ȃ��ƃt�@�C����I�������Ƃ��ɃJ�����g�f�B���N�g�����I�������t�@�C���̏ꏊ�ɕύX����Ă��܂��B
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileName(&ofn)) {
            if (engine_manager->addUPIEngine(mainWindowHandle(), ofn.lpstrFile)) {
                initEngineList();
            }
        }
    };

    commandHandler(B_DELETE_ENGINE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // ���ݑI�𒆂̃G���W������index�擾
        auto index = SendMessage(child_window[LIST_ENGINE].handle, LB_GETCURSEL, 0, 0);

        // ���X�g����폜
        SendMessage(child_window[LIST_ENGINE].handle, LB_DELETESTRING, index, 0);

        // engine.csv������폜
        engine_manager->deleteEngine(index);

        // �폜�{�^����A�N�e�B�u
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

    // ���ɓo�^����Ă���G���W����������΍폜    
    SendMessage(child_window[LIST_ENGINE].handle, LB_RESETCONTENT, 0, 0);

    // ���X�g�{�b�N�X�̏�����
    for (int i = 0; i < engine_list.size(); i++) {
        SendMessage(child_window[LIST_ENGINE].handle, LB_INSERTSTRING, i, (LPARAM)engine_list[i].engine_name.c_str());
    }
}
