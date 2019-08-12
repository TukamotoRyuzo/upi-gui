#include "window_base.h"

WindowBase::WindowBase(HINSTANCE hInst, LPSTR lpClassName) : instance_handle(hInst), class_name(lpClassName) {
}

WindowBase::~WindowBase() {
    UnregisterClass(class_name, instance_handle);
}

void WindowBase::doMessageLoop() {
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(mainWindowHandle(), 100);
}

// event_id�ɑΉ�����window�𐶐�����B
void WindowBase::create(EventID event_id) {
    ChildWindow& w = child_window.at(event_id);
    w.handle = CreateWindow(w.class_name, w.window_name, w.style, w.x, w.y, w.w, w.h,
        main_window_handle, (HMENU)event_id, instance_handle, NULL);
}

void WindowBase::createUpDown(EventID event_id, EventID bady, int upper, int lower, int now) {
    ChildWindow& w = child_window.at(event_id);
    w.handle = CreateUpDownControl(WS_CHILD | WS_VISIBLE | WS_BORDER | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
        0, 0, 0, 0, main_window_handle, event_id, instance_handle, child_window.at(bady).handle,
        upper, lower, now); 
}

// event_id�ɑΉ�����window�����B
void WindowBase::close(EventID event_id) {
    SendMessage(child_window.at(event_id).handle, WM_CLOSE, 0, 0);
}

// event_id�ɑΉ�����window��L��������B
void WindowBase::enable(EventID event_id) {
    EnableWindow(child_window.at(event_id).handle, TRUE);
}

// event_id�ɑΉ�����window�𖳌�������B
void WindowBase::disable(EventID event_id) {
    EnableWindow(child_window.at(event_id).handle, FALSE);
}

// event_id�ɑΉ�����`�F�b�N�{�b�N�X�Ƀ`�F�b�N������B
void WindowBase::check(EventID event_id) {
    SendMessage(child_window.at(event_id).handle, BM_SETCHECK, BST_CHECKED, 0);
}

// event_id�ɑΉ�����`�F�b�N�{�b�N�X�̃`�F�b�N���O���B
void WindowBase::uncheck(EventID event_id) {
    SendMessage(child_window.at(event_id).handle, BM_SETCHECK, BST_UNCHECKED, 0);
}

// event_id�ɑΉ�����`�F�b�N�{�b�N�X���`�F�b�N����Ă��邩��Ԃ��B
bool WindowBase::isChecked(EventID event_id) const {
    return SendMessage(child_window.at(event_id).handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

// �ŏ��ɐ�������window�̏�����������B
bool WindowBase::init() {
    if (!createWindow()) {
        return false;
    }

    // �E�B���h�E�v���V�[�W�����Z�b�g
    SetWindowLongPtr(main_window_handle, GWLP_USERDATA, (LONG_PTR)this);

    // WM_CREATE�𔭍s������
    SendMessage(main_window_handle, WM_CREATE, (WPARAM)0, (LPARAM)0);

    return true;
}

// �E�B���h�E�v���V�[�W���̃T�u�N���X��
// http://wisdom.sakura.ne.jp/system/winapi/win32/win64.html
// http://gomojp.blog77.fc2.com/blog-entry-92.html
LRESULT WindowBase::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowBase* instance = (WindowBase*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (instance) {
        return instance->proc(hWnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

std::function<void(HWND, UINT, WPARAM, LPARAM)>& WindowBase::commandHandler(EventID event_id) {
    return child_window.at(event_id).command_handler;
}

// �E�B���h�E�v���V�[�W��
LRESULT WindowBase::proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        setHandler();

        if (!onCreate()) {
            return false;
        }
        
        break;

    case WM_CTLCOLORSTATIC:
        return onCtlColorStatic(wParam, lParam);

    case WM_COMMAND:
        assert(child_window.find((EventID)LOWORD(wParam)) != child_window.end());
        commandHandler((EventID)LOWORD(wParam))(hWnd, uMsg, wParam, lParam);
        break;

    case WM_PAINT:
        onPaint();
        break;

    case WM_TIMER:
        onTimer();
        break;

    case WM_DESTROY:
        onDestroy();
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

void WindowBase::onDestroy() {
    PostQuitMessage(0);
}

void WindowBase::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(main_window_handle, &ps);
    EndPaint(main_window_handle, &ps);
}