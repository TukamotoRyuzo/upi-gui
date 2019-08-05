#include "pipe.h"
#include <iostream>
#include "common.h"

PipeManager::~PipeManager() {
	closeProcess();
}

// �ȉ���2�̃p�C�v���쐬����
// 1. ���v���Z�X -> �q�v���Z�X�ւ̃p�C�v
// 2. �q�v���Z�X -> ���v���Z�X�ւ̃p�C�v
int PipeManager::executeProcess(std::string engine_name) {
	process_path = engine_name;
	int ret = 0;
	try {
		HANDLE self_to_child_pipe_read_temp = INVALID_HANDLE_VALUE;
		HANDLE child_to_self_pipe_write_temp = INVALID_HANDLE_VALUE;
		HANDLE currproc = GetCurrentProcess();

		// 1. ���v���Z�X -> �q�v���Z�X�ւ̃p�C�v���쐬
		if (!CreatePipe(&self_to_child_pipe_read_temp, &self_to_child_pipe_write, NULL, 0)) {
			throw std::string("Failed to CreatePipe");
		}

		// �q�v���Z�X�̕W�����͂��쐬
		if (!DuplicateHandle(currproc, self_to_child_pipe_read_temp, currproc, &self_to_child_pipe_read, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
			throw std::string("Failed to DupliateHandle");
		}

		// 2. �q�v���Z�X -> ���v���Z�X�ւ̃p�C�v���쐬
		if (!CreatePipe(&child_to_self_pipe_read, &child_to_self_pipe_write_temp, NULL, 0)) {
			throw std::string("Failed to CreatePipe");
		}

		// �q�v���Z�X�̕W���o�͂��쐬
		if (!DuplicateHandle(currproc, child_to_self_pipe_write_temp, currproc, &child_to_self_pipe_write, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
			throw std::string("Failed to DupliateHandle");
		}

		// �q�v���Z�X�̃G���[�o�͂��쐬
		if (!DuplicateHandle(currproc, child_to_self_pipe_write, currproc, &child_error_write, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
			throw std::string("Failed to DuplicateHandle");
		}

		// temp�͍폜
		CloseHandle(self_to_child_pipe_read_temp);
		CloseHandle(child_to_self_pipe_write_temp);

		// �v���Z�X�N��
		PROCESS_INFORMATION pi = {};
		STARTUPINFOA si = {};

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;// | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdInput = self_to_child_pipe_read;   // �W�����̓n���h��
		si.hStdOutput = child_to_self_pipe_write; // �W���o�̓n���h��
		si.hStdError = child_error_write;         // �G���[�o�̓n���h��

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

		// �s�v�Ȃ�폜
		CloseHandle(pi.hThread);
		CloseHandle(self_to_child_pipe_read);
		CloseHandle(child_to_self_pipe_write);
		CloseHandle(child_error_write);
	}

	// ��O
	catch (std::string const& e) {
		OutputDebugStringA(e.c_str());
		ret = -1;
	}

	self_to_child_pipe_read = INVALID_HANDLE_VALUE;
	child_to_self_pipe_write = INVALID_HANDLE_VALUE;
	child_error_write = INVALID_HANDLE_VALUE;

	return ret;
}

// �p�C�v�ւ̏�������(�q�v���Z�X�ւ̑��M�j
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

// �p�C�v�̓ǂݍ���(�q�v���Z�X����̎�M)
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

// Process�����
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
