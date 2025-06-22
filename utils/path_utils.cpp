#include "path_utils.h"
#include <unistd.h>
#include <limits.h>
#include <filesystem>
#include <iostream>

std::string get_executable_dir() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string path(buffer);
        size_t pos = path.find_last_of("\\/");
        return (pos != std::string::npos) ? path.substr(0, pos) : "";
    }
    return "";
}

std::string resolve_path(const std::string& path) {
    if (path.empty()) return path;

    // Проверка абсолютного пути
    if (path[0] == '/') return path;

    // Проверка относительно текущей директории
    if (std::filesystem::exists(path)) return path;

    // Поиск в директории исполняемого файла
    std::string exe_dir = get_executable_dir();
    if (!exe_dir.empty()) {
        std::string exe_path = exe_dir + "/" + path;
        if (std::filesystem::exists(exe_path)) return exe_path;

        std::string parent_path = exe_dir + "/../" + path;
        if (std::filesystem::exists(parent_path)) return parent_path;

        std::string config_path = exe_dir + "/../config/" + path;
        if (std::filesystem::exists(config_path)) return config_path;
    }

    return path;
}
void print_path(const uint_fast32_t* parents, uint_fast32_t start, uint_fast32_t end) {
    if (end == start) {
        printf("%lu", start);
    } else if (parents[end] == UINT_FAST32_MAX) {
        printf("No path from %lu to %lu", start, end);
    } else {
        print_path(parents, start, parents[end]);
        printf(" -> %lu", end);
    }
}
