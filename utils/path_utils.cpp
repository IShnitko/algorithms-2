#include "path_utils.h"
#include <windows.h>
#include <filesystem>

std::string get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\/");
    return (pos != std::string::npos) ? path.substr(0, pos) : "";
}

std::string resolve_path(const std::string& path) {
    if (path.empty()) return path;

    // Если путь абсолютный
    if (path.size() > 1 && path[1] == ':') return path;

    // Проверка относительно текущей директории
    if (std::filesystem::exists(path)) return path;

    // Проверка относительно директории исполняемого файла
    std::string exe_dir = get_executable_dir();
    if (!exe_dir.empty()) {
        std::string exe_path = exe_dir + "\\" + path;
        if (std::filesystem::exists(exe_path)) return exe_path;

        std::string parent_path = exe_dir + "\\..\\" + path;
        if (std::filesystem::exists(parent_path)) return parent_path;
    }

    return path;
}
