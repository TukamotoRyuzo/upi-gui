#include "degug_window.h"
#include "common.h"

DebugWindow::DebugWindow(HINSTANCE hInst, HWND hWnd, PipeManager* p1, PipeManager* p2) :
	WindowBase(hInst, "デバッグログ"),
	parent_window_handle(hWnd),
	pipe1(p1),
	pipe2(p2) {
}

bool DebugWindow::createWindow() {
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
	RECT r = { 0, 0, 700, 350 };

	// ウィンドウサイズの自動計算
	AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, false, 0);

	// ウィンドウ生成、表示
	main_window_handle = CreateWindow(class_name, class_name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
		NULL, NULL, instance_handle, NULL);
	
	return main_window_handle != NULL;
}

// 親ウィンドウ側で、デバッグウィンドウが死んだことを検知するためのメッセージをポストしておくか
void DebugWindow::onDestroy() {
	Log::setDebugWindow(NULL);
	PostMessage(parent_window_handle, WM_COMMAND, DEBUG_WINDOW_DESTROYED, 0);
}

bool DebugWindow::onCreate() {
	const DWORD style_button = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
	const DWORD style_edit = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY |
		ES_LEFT | ES_MULTILINE;

	// button
	child_window[B_SEND_MESSAGE_TO_ENGINE1] = ChildWindow(TEXT("BUTTON"), TEXT("エンジン1に送信"), style_button, 300, 310, 140, 30);
	child_window[B_SEND_MESSAGE_TO_ENGINE2] = ChildWindow(TEXT("BUTTON"), TEXT("エンジン2に送信"), style_button, 450, 310, 140, 30);
	child_window[B_DELETE_LOG] = ChildWindow(TEXT("BUTTON"), TEXT("ログ消去"), style_button, 600, 310, 80, 30);

	// edit
	child_window[EDIT_DEBUG_LOG] = ChildWindow(TEXT("edit"), NULL, style_edit, 0, 0, 700, 300);
	child_window[EDIT_SEND_COMMAND] = ChildWindow(TEXT("edit"), NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 30, 310, 260, 30);

	create(B_SEND_MESSAGE_TO_ENGINE1);
	create(B_SEND_MESSAGE_TO_ENGINE2);
	create(B_DELETE_LOG);
	create(EDIT_DEBUG_LOG);
	create(EDIT_SEND_COMMAND);

	// エディットコントロールの上限文字数はデフォルトでは32KBらしいので1GBにする。
	SendMessage(child_window.at(EDIT_DEBUG_LOG).handle, EM_SETLIMITTEXT, (WPARAM)1024 * 1024 * 1024, 0);

	// Logに紐付ける
	Log::setDebugWindow(child_window.at(EDIT_DEBUG_LOG).handle);

	InvalidateRect(main_window_handle, NULL, false);
	ShowWindow(main_window_handle, SW_SHOW);
	return true;
}

void DebugWindow::setHandler() {
	commandHandler(B_SEND_MESSAGE_TO_ENGINE1) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (pipe1->connecting()) {
			int count = GetWindowTextLength(child_window.at(EDIT_SEND_COMMAND).handle);
			if (count) {
				char* buf = new char[count + 1];
				GetWindowText(child_window.at(EDIT_SEND_COMMAND).handle, buf, count + 1);
				pipe1->sendMessage(std::string(buf));
			}
		}
	};

	commandHandler(B_SEND_MESSAGE_TO_ENGINE2) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (pipe2->connecting()) {
			int count = GetWindowTextLength(child_window.at(EDIT_SEND_COMMAND).handle);
			if (count) {
				char* buf = new char[count + 1];
				GetWindowText(child_window.at(EDIT_SEND_COMMAND).handle, buf, count + 1);
				pipe2->sendMessage(std::string(buf));
			}
		}
	};

	commandHandler(B_DELETE_LOG) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		Log::deleteLog();
	};

	commandHandler(EDIT_DEBUG_LOG) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		
	};

	commandHandler(EDIT_SEND_COMMAND) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	};
}
