#include <ctime>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include "pipe.h"
#include "main_window.h"
#include "manage_engine_window.h"
#include "degug_window.h"

MainWindow::MainWindow(HINSTANCE hInst) : WindowBase(hInst, TEXT("upi-gui")), game(this) {

}

// ウィンドウ生成
bool MainWindow::createWindow() {
	// windowクラス登録
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WindowBase::windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance_handle;
	wc.hIcon = LoadIcon(instance_handle, TEXT("IDI_ICON1"));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = TEXT("KITTY");
	wc.lpszClassName = class_name;
	wc.hIconSm = LoadIcon(instance_handle, TEXT("IDI_ICON1"));

	if (!RegisterClassEx(&wc)) {
		return false;
	}

	// windowサイズ設定
	RECT r = { 0, 0, WIN_WIDTH, WIN_HEIGHT };

	// ウィンドウサイズの自動計算
	AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, true, 0);

	// ウィンドウ生成、表示
	main_window_handle = CreateWindow(class_name, class_name,
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
		NULL, NULL, instance_handle, NULL);

	return main_window_handle != NULL;
}

// WM_CREATE発行時(ウィンドウ生成時)に行われる処理
bool MainWindow::onCreate() {
	const DWORD style_button = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
	const DWORD style_check = WS_CHILD | WS_VISIBLE | BS_CHECKBOX;
	const DWORD style_combo = WS_CHILD | WS_VISIBLE | CBS_SORT | CBS_DROPDOWNLIST;

	// button
	child_window[B_START] = ChildWindow(TEXT("BUTTON"), TEXT("start"), style_button, 440, 300, 100, 50);
	child_window[B_STOP] = ChildWindow(TEXT("BUTTON"), TEXT("■"), style_button, 475, 380, 30, 15);
	child_window[B_RESTART] = ChildWindow(TEXT("BUTTON"), TEXT("▲"), style_button, 475, 380, 30, 15);

	// combobox
	child_window[COMBO_AI1P] = ChildWindow(TEXT("COMBOBOX"), NULL, style_combo | WS_DISABLED, 375, 580, 100, 600);
	child_window[COMBO_AI2P] = ChildWindow(TEXT("COMBOBOX"), NULL, style_combo | WS_DISABLED, 500, 580, 100, 600);

	// checkbox
	child_window[CHECK_AI1P] = ChildWindow(TEXT("BUTTON"), TEXT("AI"), style_check, 433, 540, 40, 30);
	child_window[CHECK_AI2P] = ChildWindow(TEXT("BUTTON"), TEXT("AI"), style_check, 500, 540, 40, 30);
	child_window[CHECK_CONTINUE_BATTLE] = ChildWindow(TEXT("BUTTON"), TEXT("Continuous"), style_check, 433, 490, 130, 30);
	child_window[CHECK_PLAY_SOUND] = ChildWindow(TEXT("BUTTON"), TEXT("Voice"), style_check, 433, 515, 130, 30);

	// menuは皮だけ作っておこう
	for (EventID eid = MOK_START; eid < MOK_END; eid = EventID(eid + 1)) {
		child_window[eid] = ChildWindow();
	}

	// bitmap
	mdc[MEMORY] = BitmapMDC();
	mdc[PUYO] = BitmapMDC("IDB_BITMAP3");
	mdc[FIELD] = BitmapMDC("IDB_BITMAP1");

	// 音声ファイルの読み込み
	if (!initMCI()) {
		return false;
	}

	// 画像の読み込み
	if (!initDC()) {
		return false;
	}

	create(B_START);
	create(COMBO_AI1P);
	create(COMBO_AI2P);
	create(CHECK_AI1P);
	create(CHECK_AI2P);
	create(CHECK_CONTINUE_BATTLE);
	create(CHECK_PLAY_SOUND);
	show(FIELD);
	game.init();
	game.show();
	initEngineList();
	InvalidateRect(main_window_handle, NULL, false);
	ShowWindow(main_window_handle, SW_SHOW);
	return true;
}

// 音声ファイルの初期化をする。
bool MainWindow::initMCI() {
    int result;

	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < 11; i++) {
			char buf[100];
			sprintf_s(buf, 100, "./voice/%dp/ren%d.mp3", p + 1, i + 1);
			ren_voice[p][i].lpstrDeviceType = "MPEGVideo";
			ren_voice[p][i].lpstrElementName = buf;
			if (result = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)& ren_voice[p][i])) {
				char message[1000];
				mciGetErrorString(result, message, sizeof(message));
				MessageBox(NULL, message, buf, MB_OK);
				PostQuitMessage(0);
				return false;
			}
		}
	}

    return true;
}

// ダブルバッファで描画するための準備を行う。
bool MainWindow::initDC() {
    HDC hdc = GetDC(main_window_handle);

    for (BitmapID i = MEMORY; i < BITMAP_NB; i = BitmapID(i + 1)) {
        mdc[i].handle = CreateCompatibleDC(hdc);
        HBITMAP hb = i == MEMORY ? CreateCompatibleBitmap(hdc, WIN_WIDTH, WIN_HEIGHT)
                                 : (HBITMAP)LoadBitmap(instance_handle, mdc[i].bitmap_name);
		if (hb == NULL) {
			return false;
		}

        mdc[i].prev = (HBITMAP)SelectObject(mdc[i].handle, hb);
    }

    SetBkMode(mdc[MEMORY].handle, TRANSPARENT);
    ReleaseDC(main_window_handle, hdc);
    return true;
}

// upiエンジンのリスト(engine_list.csv)を読み込み、コンボボックスに反映させる。
bool MainWindow::initEngineList() {
	engine_manager.initEngineList();
	auto engine_list = engine_manager.getUPIEngineList();

	// リストから削除
	SendMessage(child_window[COMBO_AI1P].handle, CB_RESETCONTENT, 0, 0);
	SendMessage(child_window[COMBO_AI2P].handle, CB_RESETCONTENT, 0, 0);

	// コンボボックスの初期化
	for (int i = 0; i < engine_list.size(); i++) {
		SendMessage(child_window[COMBO_AI1P].handle, CB_INSERTSTRING, i, (LPARAM)engine_list[i].engine_name.c_str());
		SendMessage(child_window[COMBO_AI2P].handle, CB_INSERTSTRING, i, (LPARAM)engine_list[i].engine_name.c_str());
	}

	// 最初のやつを選んでおく
	SendMessage(child_window[COMBO_AI1P].handle, CB_SETCURSEL, 0, 0);
	SendMessage(child_window[COMBO_AI2P].handle, CB_SETCURSEL, 0, 0);

	// あれ、falseなくない
	return true;
}

// 終了処理
void MainWindow::onDestroy() {
	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < 11; i++) {
			mciSendCommand(ren_voice[p][i].wDeviceID, MCI_CLOSE, 0, 0);
		}
	}

	for (BitmapID i = MEMORY; i < BITMAP_NB; i = BitmapID(i + 1)) {
        auto& m = mdc[i];
        HBITMAP hb = (HBITMAP)SelectObject(m.handle, m.prev);
        DeleteObject(hb);
        DeleteObject(m.handle);
    }

	if (debug_window) {
		SendMessage(debug_window->mainWindowHandle(), WM_CLOSE, 0, 0);
		delete debug_window;
	}

	WindowBase::onDestroy();
}

// WINAPIの慣例に従って描画する。
void MainWindow::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(main_window_handle, &ps);
    BitBlt(hdc, 0, 0, WIN_WIDTH, WIN_HEIGHT, mdc[MEMORY].handle, 0, 0, SRCCOPY);
    EndPaint(main_window_handle, &ps);
}

void MainWindow::playVoice(int player, int chain) const {
	if (isChecked(CHECK_PLAY_SOUND)) {
		auto m = mainWindowHandle();
		mciSendCommand(ren_voice[player][chain].wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)& m);
	}
}

HDC MainWindow::dcHandle(BitmapID bid) const {
    return mdc.at(bid).handle;
}

void MainWindow::show(BitmapID bid) const {
    BitBlt(mdc.at(MEMORY).handle, 0, 0, WIN_WIDTH, WIN_HEIGHT, mdc.at(bid).handle, 0, 0, SRCCOPY);
}

// イベントハンドラを定義する。
void MainWindow::setHandler() {
    commandHandler(B_START) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // "start"を押したときに無効にするボタンたち
		disable(COMBO_AI1P);
		disable(COMBO_AI2P);
		disable(CHECK_AI1P);
		disable(CHECK_AI2P);
        close(B_STOP);
        close(B_RESTART);
        create(B_STOP);

		// ゲーム開始
        game.init();
		SetTimer(mainWindowHandle(), 100, 10, NULL);

		UPIManager upi;

		// AIにチェックが入っていればエンジンを起動する。
		auto launchEngine = [&](EventID eid, PipeManager& pm) {
			int engine_id = SendMessage(child_window[eid].handle, CB_GETCURSEL, 0, 0);			
			pm.executeProcess(engine_manager.getUPIEngineList().at(engine_id).engine_path);			
			upi.launchEngine(pm, game.getTumo());
		};

		if (game.p1.status & PLAYER_AI) {
			launchEngine(COMBO_AI1P, game.p1.pipe);
		}
		if (game.p2.status & PLAYER_AI) {
			launchEngine(COMBO_AI2P, game.p2.pipe);
		}
    };

    commandHandler(B_STOP) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        game.stop();
        close(B_STOP);
        create(B_RESTART);
        KillTimer(main_window_handle, 100);
		enable(CHECK_AI1P);
		enable(CHECK_AI2P);

		if (isChecked(CHECK_AI1P)) {
			enable(COMBO_AI1P);
		}
		if (isChecked(CHECK_AI2P)) {
			enable(COMBO_AI2P);
		}
    };

    commandHandler(B_RESTART) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        game.restart();
        close(B_RESTART);
        create(B_STOP);
        SetTimer(main_window_handle, 100, 10, NULL);
		disable(COMBO_AI1P);
		disable(COMBO_AI2P);
    };

    // combobox
    commandHandler(COMBO_AI1P) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // AIのレベルが変更されたとき
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            // 選択されたindexを取得
            //Game::level_1p = SendMessage(window[COMBO_AI1P].handle, CB_GETCURSEL, 0, 0) + 1;
            //Game::ai3con.depthMax = Game::level_1p;// aiの探索深さを決定する。
        }
    };

    commandHandler(COMBO_AI2P) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            //Game::level_2p = SendMessage(window[COMBO_AI2P].handle, CB_GETCURSEL, 0, 0) + 1;
            //Game::ai3con2.depthMax = Game::level_2p;
        }
    };

    // checkbox
    commandHandler(CHECK_AI1P) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // checkなら非check,非checkならcheckする
        if (isChecked(CHECK_AI1P)) {
            game.p1.status &= ~PLAYER_AI;
			uncheck(CHECK_AI1P);
			disable(COMBO_AI1P);
        }
        else {
			game.p1.status |= PLAYER_AI;
			check(CHECK_AI1P);
			enable(COMBO_AI1P);
        }
    };

    commandHandler(CHECK_AI2P) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // checkなら非check,非checkならcheckする
        if (isChecked(CHECK_AI2P)) {
			game.p2.status &= ~PLAYER_AI;
			uncheck(CHECK_AI2P);
			disable(COMBO_AI2P);
        }
        else {
			game.p2.status |= PLAYER_AI;
			check(CHECK_AI2P);
			enable(COMBO_AI2P);
        }
    };

	// checkbox
	commandHandler(CHECK_CONTINUE_BATTLE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		// checkなら非check,非checkならcheckする
		if (isChecked(CHECK_CONTINUE_BATTLE)) {
			uncheck(CHECK_CONTINUE_BATTLE);
		}
		else {
			check(CHECK_CONTINUE_BATTLE);
		}
	};

	commandHandler(CHECK_PLAY_SOUND) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		// checkなら非check,非checkならcheckする
		if (isChecked(CHECK_PLAY_SOUND)) {
			uncheck(CHECK_PLAY_SOUND);
		}
		else {
			check(CHECK_PLAY_SOUND);
		}
	};

	commandHandler(MENU_ADD_ENGINE) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		ManageEngineWindow mew(instance_handle, &engine_manager);

		if (!mew.init()) {
			onDestroy();
			return;
		}

		EnableWindow(main_window_handle, false);
		mew.doMessageLoop();
		EnableWindow(main_window_handle, true);
		SetFocus(main_window_handle);
		initEngineList();
	};

	commandHandler(MENU_DEBUG_LOG) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (debug_window == nullptr) {
			debug_window = new DebugWindow(instance_handle, mainWindowHandle(), &game.p1.pipe, &game.p2.pipe);

			if (!debug_window->init()) {
				onDestroy();
				return;
			}
		}
		else {
			DestroyWindow(debug_window->mainWindowHandle());
		}
	};

	commandHandler(DEBUG_WINDOW_DESTROYED) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		delete debug_window;
		debug_window = nullptr;
	};

	commandHandler(MENU_END) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		onDestroy();
	};

	commandHandler(MENU_VERSION) = [&](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		MessageBox(mainWindowHandle(), "1.0.0", "バージョン情報", MB_OK);
	};
}

void MainWindow::onMCINotify(WPARAM wParam, LPARAM lParam) {
	if (wParam == MCI_NOTIFY_SUCCESSFUL) {
		mciSendCommand(lParam, MCI_SEEK, MCI_SEEK_TO_START, 0);
	}
}

LRESULT MainWindow::onCtlColorStatic(WPARAM wParam, LPARAM lParam) {
	HBRUSH hbkgnd = NULL;

	// チェックボックス描画時に背景を透過させる
	if (GetDlgCtrlID((HWND)lParam) == CHECK_AI1P
		|| GetDlgCtrlID((HWND)lParam) == CHECK_AI2P
		|| GetDlgCtrlID((HWND)lParam) == CHECK_CONTINUE_BATTLE
		|| GetDlgCtrlID((HWND)lParam) == CHECK_PLAY_SOUND
		) {
		SetBkMode((HDC)wParam, TRANSPARENT); // 背景を透過

		if (!hbkgnd) {
			LOGBRUSH lb;
			lb.lbStyle = BS_HOLLOW;
			hbkgnd = CreateBrushIndirect(&lb);
		}
	}

	return((LRESULT)hbkgnd);
}

void MainWindow::onTimer() {
	game.update();
}

void MainWindow::gameOver() {
	if (!isChecked(CHECK_CONTINUE_BATTLE)) {
		KillTimer(main_window_handle, 100);
		close(B_STOP);
		enable(CHECK_AI1P);
		enable(CHECK_AI2P);

		if (isChecked(CHECK_AI1P)) {
			enable(COMBO_AI1P);
		}

		if (isChecked(CHECK_AI2P)) {
			enable(COMBO_AI2P);
		}
	}
	else {
		commandHandler(B_START)((HWND)0, (UINT)0, (WPARAM)0, (LPARAM)0);
	}
}

void MainWindow::drawString(RECT* rect, TCHAR* str, UINT format) {
    BitBlt(dcHandle(MEMORY), rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, dcHandle(FIELD), rect->left, rect->top, SRCCOPY);
    DrawText(dcHandle(MEMORY), str, -1, rect, format);
    InvalidateRect(main_window_handle, rect, false);
}

// 指定した範囲をもとのfield画像に戻す
void MainWindow::rectClear(RECT *rect) {
    BitBlt(dcHandle(MEMORY), rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, dcHandle(FIELD), rect->left, rect->top, SRCCOPY);
    InvalidateRect(main_window_handle, rect, false);
}
