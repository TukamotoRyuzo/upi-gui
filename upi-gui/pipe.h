#pragma once

#include <windows.h>
#include <string>
#include <fstream>

class PipeManager {    
    HANDLE child_process = INVALID_HANDLE_VALUE;
    HANDLE child_error_write = INVALID_HANDLE_VALUE;
    HANDLE self_to_child_pipe_write = INVALID_HANDLE_VALUE;
    HANDLE child_to_self_pipe_read = INVALID_HANDLE_VALUE;
    HANDLE self_to_child_pipe_read = INVALID_HANDLE_VALUE;
    HANDLE child_to_self_pipe_write = INVALID_HANDLE_VALUE;
    std::string process_path;
public:
    ~PipeManager();
    int executeProcess(std::string process_name);
    int sendMessage(std::string message);
    int recvMessage(std::string& message);
    int closeProcess();
    bool connecting() const;
    std::string processPath() const;
};
