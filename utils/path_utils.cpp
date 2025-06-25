#include "path_utils.h" // Include the header file for path utility functions.
#include <unistd.h>     // Required for readlink (for Unix-like systems, to get executable path).
#include <limits.h>     // Required for PATH_MAX (maximum length of a file path).
#include <filesystem>   // C++17 filesystem library for path manipulation and existence checks.
#include <iostream>     // For std::cout, std::cerr (used for debugging or general output).
#include <cstdio>       // For printf, used in print_path.
#include <cstdint>      // For UINT_FAST32_MAX.

// Function to get the directory of the current executable.
// This implementation is specific to Linux using /proc/self/exe.
// Returns: A std::string containing the absolute path to the executable's directory, or an empty string on failure.
std::string get_executable_dir() {
    char buffer[PATH_MAX]; // Buffer to store the executable path.
    // readlink("/proc/self/exe", ...) gets the path to the current executable.
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) { // If readlink was successful.
        buffer[len] = '\0'; // Null-terminate the buffer.
        std::string path(buffer); // Convert C-string to std::string.
        // Find the last path separator ('\' or '/').
        size_t pos = path.find_last_of("\\/");
        // If a separator is found, return the substring before it (the directory).
        return (pos != std::string::npos) ? path.substr(0, pos) : "";
    }
    // If readlink failed, return an empty string.
    return "";
}

// Function to resolve a given path to an absolute or existing relative path.
// It checks in various locations: as an absolute path, relative to current directory,
// relative to the executable's directory, and common subdirectories like "../" and "../config/".
// path: The input path string.
// Returns: The resolved absolute path if found, or the original path if no resolution succeeds.
std::string resolve_path(const std::string& path) {
    if (path.empty()) return path; // Handle empty path.

    // 1. Check if it's already an absolute path (starts with '/').
    if (path[0] == '/') return path;

    // 2. Check relative to the current working directory.
    // std::filesystem::exists checks if a file or directory exists at the given path.
    if (std::filesystem::exists(path)) return path;

    // 3. Search in the executable's directory and common relative paths.
    std::string exe_dir = get_executable_dir();
    if (!exe_dir.empty()) {
        // Try path relative to executable's directory directly.
        std::string exe_path = exe_dir + "/" + path;
        if (std::filesystem::exists(exe_path)) return exe_path;

        // Try path relative to parent of executable's directory (e.g., if executable is in 'bin/').
        std::string parent_path = exe_dir + "/../" + path;
        if (std::filesystem::exists(parent_path)) return parent_path;

        // Try path relative to a 'config' subdirectory relative to parent of executable's directory.
        std::string config_path = exe_dir + "/../config/" + path;
        if (std::filesystem::exists(config_path)) return config_path;
    }

    // If no resolution succeeds, return the original path.
    // The caller should handle this (e.g., report file not found).
    return path;
}

// Recursively prints the path from a start vertex to an end vertex using a parent array.
// parents: An array where parents[i] stores the predecessor of vertex i in the path.
// start: The starting vertex of the path.
// end: The ending vertex of the path.
void print_path(const uint_fast32_t* parents, uint_fast32_t start, uint_fast32_t end) {
    // Base case 1: If start and end are the same, we've reached the beginning of the path.
    if (end == start) {
        printf("%lu", start); // Print the starting vertex.
    }
    // Base case 2: If the 'end' vertex has no parent (meaning it's unreachable or the source itself).
    // UINT_FAST32_MAX is used as a sentinel value for no parent/unreachability.
    else if (parents[end] == UINT_FAST32_MAX) {
        printf("No path from %lu to %lu", start, end); // Indicate no path exists.
    }
    // Recursive step: If 'end' has a parent, print the path to its parent first.
    else {
        print_path(parents, start, parents[end]); // Recursively call for the parent.
        printf(" -> %lu", end); // Then print the current 'end' vertex, forming the path.
    }
}
