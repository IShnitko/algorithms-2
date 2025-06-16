#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>

std::string get_executable_dir();
std::string resolve_path(const std::string& path);

#endif // PATH_UTILS_H