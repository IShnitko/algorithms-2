#include "path_utils.h"
#include <windows.h>
#include <filesystem>
#include <iostream>

std::string get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\/");
    return (pos != std::string::npos) ? path.substr(0, pos) : "";
}

std::string resolve_path(const std::string& path) {
    if (path.empty()) return path;
    
    // Проверка абсолютного пути
    if (path.size() > 1 && path[1] == ':') return path;
    
    // Проверка относительно текущей директории
    if (std::filesystem::exists(path)) return path;
    
    // Поиск в ../config/ относительно директории исполняемого файла
    std::string exe_dir = get_executable_dir();
    if (!exe_dir.empty()) {
        // ../config/file_name
        std::string config_path = exe_dir + "\\..\\config\\" + path;
        if (std::filesystem::exists(config_path)) {
            return config_path;
        }
        
        // ../file_name
        std::string parent_path = exe_dir + "\\..\\" + path;
        if (std::filesystem::exists(parent_path)) {
            return parent_path;
        }
        
        // Прямо в директории исполняемого файла
        std::string exe_path = exe_dir + "\\" + path;
        if (std::filesystem::exists(exe_path)) {
            return exe_path;
        }
    }
    
    std::cerr << "Warning: File not found at any location: " << path << std::endl;
    return path;
}
void print_path(const uint_fast32_t* parents, uint_fast32_t start, uint_fast32_t end) {
    if (end == start) {
        printf("%u", start);
    } else if (parents[end] == UINT32_MAX) {
        printf("No path from %u to %u", start, end);
    } else {
        print_path(parents, start, parents[end]);
        printf(" -> %u", end);
    }
}
