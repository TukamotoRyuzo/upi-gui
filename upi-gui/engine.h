#pragma once

#include <string>
#include <vector>
#include <windows.h>

struct EngineInfo {
    std::string engine_name = "", engine_path = "", engine_author = "";
    bool isNull() const {
        return engine_name == "" || engine_path == "" || engine_author == "";
    }
    std::string toCSV() {
        return engine_name + "," + engine_path + "," + engine_author;
    }
};

class MainWindow;

class UPIEngineManager {
public:
    bool registerEngine(EngineInfo ei);
    bool initEngineList();
    bool addUPIEngine(HWND hwnd, std::string engine_path);
    void deleteEngine(int index);
    const std::vector<EngineInfo>& getUPIEngineList() const {
        return engine_list;
    }    
private:
    void writeEngineToCSV();
    const std::string engine_list_file = "engine_list.csv";    
    std::vector<EngineInfo> engine_list;
};