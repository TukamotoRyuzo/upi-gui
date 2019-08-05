#include "engine.h"
#include "main_window.h"

// engine_list.csvからUPIエンジンのリストを読み出し、engine_listを初期化する。
bool UPIEngineManager::initEngineList() {
	engine_list.clear();
	std::fstream fs(engine_list_file, std::ios::in);

	// ファイルが見つからない場合はまだエンジンが登録されていないということなので、正常。
	if (fs.fail()) {
		return true;
	}

	std::string line = "";
	EngineInfo ei;

	// csvをパースしてエンジンリストを作成
	while (std::getline(fs, line)) {
		std::istringstream iss(line);
		std::getline(iss, ei.engine_name, ',');
		std::getline(iss, ei.engine_path, ',');
		std::getline(iss, ei.engine_author, ',');
		engine_list.push_back(ei);
	}

	return true;
}

// engine_pathで指定された実行ファイルを起動し、「upi」コマンドを送る。
// upiエンジンであればengine_list.csvに登録する。
bool UPIEngineManager::addUPIEngine(HWND hwnd, std::string engine_path) {
	PipeManager pm;
	UPIManager upi;

	if (pm.executeProcess(engine_path) != 0) {
		MessageBox(hwnd, "ファイルが見つかりません。", "failed", MB_OK);
		return false;
	}
	else {
		upi.upi(pm);
		EngineInfo ei = upi.id(pm);

		if (ei.isNull()) {
			MessageBox(hwnd, "UPIエンジンではありません。", "failed", MB_OK);
			return false;
		}
		else if (registerEngine(ei)) {
			MessageBox(hwnd, (ei.engine_name + "を登録しました。").c_str(), "success", MB_OK);
			return true;
		}
		else {
			MessageBox(hwnd, "すでに登録されています。", "failed", MB_OK);
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

// エンジン名、エンジンのパス、エンジンの作者の組をengine_list.csvに登録する。
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
