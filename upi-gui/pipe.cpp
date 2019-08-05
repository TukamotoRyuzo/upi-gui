#include "pipe.h"
#include <iostream>
#include "common.h"

PipeManager::~PipeManager() {
	closeProcess();
}

// 以下の2つのパイプを作成する
// 1. 自プロセス -> 子プロセスへのパイプ
// 2. 子プロセス -> 自プロセスへのパイプ
int PipeManager::executeProcess(std::string engine_name) {
	process_path = engine_name;
	int ret = 0;
	try {
		HANDLE self_to_child_pipe_read_temp = INVALID_HANDLE_VALUE;
		HANDLE child_to_self_pipe_write_temp = INVALID_HANDLE_VALUE;
		HANDLE currproc = GetCurrentProcess();

		// 1. 自プロセス -> 子プロセスへのパイプを作成
		if (!CreatePipe(&self_to_child_pipe_read_temp, &self_to_child_pipe_write, NULL, 0)) {
			throw std::string("Failed to CreatePipe");
		}

		// 子プロセスの標準入力を作成
		if (!DuplicateHandle(currproc, self_to_child_pipe_read_temp, currproc, &self_to_child_pipe_read, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
			throw std::string("Failed to DupliateHandle");
		}

		// 2. 子プロセス -> 自プロセスへのパイプを作成
		if (!CreatePipe(&child_to_self_pipe_read, &child_to_self_pipe_write_temp, NULL, 0)) {
			throw std::string("Failed to CreatePipe");
		}

		// 子プロセスの標準出力を作成
		if (!DuplicateHandle(currproc, child_to_self_pipe_write_temp, currproc, &child_to_self_pipe_write, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
			throw std::string("Failed to DupliateHandle");
		}

		// 子プロセスのエラー出力を作成
		if (!DuplicateHandle(currproc, child_to_self_pipe_write, currproc, &child_error_write, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
			throw std::string("Failed to DuplicateHandle");
		}

		// tempは削除
		CloseHandle(self_to_child_pipe_read_temp);
		CloseHandle(child_to_self_pipe_write_temp);

		// プロセス起動
		PROCESS_INFORMATION pi = {};
		STARTUPINFOA si = {};

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;// | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdInput = self_to_child_pipe_read;   // 標準入力ハンドル
		si.hStdOutput = child_to_self_pipe_write; // 標準出力ハンドル
		si.hStdError = child_error_write;         // エラー出力ハンドル

		if (si.hStdOutput == INVALID_HANDLE_VALUE) {
			throw std::string("GetStdHandle(STD_ERROR_HANDLE)");
		}
		
		if (si.hStdError == INVALID_HANDLE_VALUE) {
			throw std::string("GetStdHandle(STD_ERROR_HANDLE)");
		}

		if (!CreateProcess(0, (LPSTR)engine_name.c_str(), 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
			throw std::string("Failed to CreateProcess");
		}

		child_process = pi.hProcess;

		// 不要なやつ削除
		CloseHandle(pi.hThread);
		CloseHandle(self_to_child_pipe_read);
		CloseHandle(child_to_self_pipe_write);
		CloseHandle(child_error_write);
	}

	// 例外
	catch (std::string const& e) {
		OutputDebugStringA(e.c_str());
		ret = -1;
	}

	self_to_child_pipe_read = INVALID_HANDLE_VALUE;
	child_to_self_pipe_write = INVALID_HANDLE_VALUE;
	child_error_write = INVALID_HANDLE_VALUE;

	return ret;
}

// パイプへの書き込み(子プロセスへの送信）
int PipeManager::sendMessage(std::string message) {
	DWORD numberOfBytesWritten;	
	message += "\r\n";	
	Log::write(">> " + message);

	if (!WriteFile(self_to_child_pipe_write, (LPSTR)message.c_str(), message.length(), &numberOfBytesWritten, NULL)) {
		printf("WriteFile");
		return -1;
	}

	return 0;
}

// パイプの読み込み(子プロセスからの受信)
int PipeManager::recvMessage(std::string &message) {
	DWORD numberOfBytesRead;
	char buf[256];
	
	if (!ReadFile(child_to_self_pipe_read, buf, sizeof(buf) - 1, &numberOfBytesRead, NULL)) {
		if (GetLastError() == ERROR_BROKEN_PIPE) {
			printf("pipe end\n");
		}
		printf("ReadFile");
		return -1;
	}

	buf[numberOfBytesRead] = '\0';
	message = std::string(buf);
	Log::write("<< " + message);
	return 0;
}

// Processを閉じる
int PipeManager::closeProcess() {
	auto closer = [](HANDLE h) {
		if (h != INVALID_HANDLE_VALUE) {
			if (!CloseHandle(h)) {
				throw std::runtime_error("CloseHandle");
			}
		}
	};
	
	int ret = 0;

	try {
		closer(child_process);
		closer(self_to_child_pipe_read);
		closer(child_to_self_pipe_write);
		closer(child_error_write);
		closer(self_to_child_pipe_write);
		closer(child_to_self_pipe_read);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		ret = -1;
	}

	return ret;
}

bool PipeManager::connecting() const {
	return child_process != INVALID_HANDLE_VALUE;
}

std::string PipeManager::processPath() const {
	return process_path;
}
