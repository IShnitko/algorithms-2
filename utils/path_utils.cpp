#include "path_utils.h"
#include <windows.h>
#include <filesystem>
#include <iostream>

// Get the directory of the current executable
std::string get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);  // Get full path to executable
    std::string path(buffer);
    size_t pos = path.find_last_of("\\/");       // Find last directory separator
    return (pos != std::string::npos) ? path.substr(0, pos) : "";
}

// Resolve a file path by checking various locations relative to executable
std::string resolve_path(const std::string& path) {
    if (path.empty()) return path;

    // Check if path is absolute (e.g., C:\something)
    if (path.size() > 1 && path[1] == ':') return path;

    // Check if file exists relative to current working directory
    if (std::filesystem::exists(path)) return path;

    // Try locating file relative to executable directory
    std::string exe_dir = get_executable_dir();
    if (!exe_dir.empty()) {
        // Check ../config/<filename>
        std::string config_path = exe_dir + "\\..\\config\\" + path;
        if (std::filesystem::exists(config_path)) {
            return config_path;
        }

        // Check ../<filename>
        std::string parent_path = exe_dir + "\\..\\" + path;
        if (std::filesystem::exists(parent_path)) {
            return parent_path;
        }

        // Check directly in executable directory
        std::string exe_path = exe_dir + "\\" + path;
        if (std::filesystem::exists(exe_path)) {
            return exe_path;
        }
    }

    // If not found anywhere, print warning and return original path
    std::cerr << "Warning: File not found at any location: " << path << std::endl;
    return path;
}

// Recursively print path from start to end using the parents array
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
