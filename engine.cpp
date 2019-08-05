#include "engine.h"
#include "main_window.h"

// engine_list.csv����UPI�G���W���̃��X�g��ǂݏo���Aengine_list������������B
bool UPIEngineManager::initEngineList() {
	engine_list.clear();
	std::fstream fs(engine_list_file, std::ios::in);

	// �t�@�C����������Ȃ��ꍇ�͂܂��G���W�����o�^����Ă��Ȃ��Ƃ������ƂȂ̂ŁA����B
	if (fs.fail()) {
		return true;
	}

	std::string line = "";
	EngineInfo ei;

	// csv���p�[�X���ăG���W�����X�g���쐬
	while (std::getline(fs, line)) {
		std::istringstream iss(line);
		std::getline(iss, ei.engine_name, ',');
		std::getline(iss, ei.engine_path, ',');
		std::getline(iss, ei.engine_author, ',');
		engine_list.push_back(ei);
	}

	return true;
}

// engine_path�Ŏw�肳�ꂽ���s�t�@�C�����N�����A�uupi�v�R�}���h�𑗂�B
// upi�G���W���ł����engine_list.csv�ɓo�^����B
bool UPIEngineManager::addUPIEngine(HWND hwnd, std::string engine_path) {
	PipeManager pm;
	UPIManager upi;

	if (pm.executeProcess(engine_path) != 0) {
		MessageBox(hwnd, "�t�@�C����������܂���B", "failed", MB_OK);
		return false;
	}
	else {
		upi.upi(pm);
		EngineInfo ei = upi.id(pm);

		if (ei.isNull()) {
			MessageBox(hwnd, "UPI�G���W���ł͂���܂���B", "failed", MB_OK);
			return false;
		}
		else if (registerEngine(ei)) {
			MessageBox(hwnd, (ei.engine_name + "��o�^���܂����B").c_str(), "success", MB_OK);
			return true;
		}
		else {
			MessageBox(hwnd, "���łɓo�^����Ă��܂��B", "failed", MB_OK);
			return false;
		}
	}
}

void UPIEngineManager::deleteEngine(int index) {
	engine_list.erase(engine_list.begin() + index);
	writeEngineToCSV();
}

void UPIEngineManager::writeEngineToCSV() {
	std::fstream fs(engine_list_file, std::ios::out);
	for (auto ei : engine_list) {
		fs << ei.toCSV() << std::endl;
	}
	fs.close();
}

// �G���W�����A�G���W���̃p�X�A�G���W���̍�҂̑g��engine_list.csv�ɓo�^����B
bool UPIEngineManager::registerEngine(EngineInfo ei) {
	std::fstream fs(engine_list_file, std::ios::in);

	if (!fs.fail()) {
		for (std::string line = ""; std::getline(fs, line); ) {
			if (ei.toCSV() == line) {
				return false;
			}
		}
	}

	fs.close();
	fs.open(engine_list_file, std::ios::out | std::ios::app);
	fs << ei.toCSV() << std::endl;
	fs.close();
	return true;
}
