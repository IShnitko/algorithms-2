// path_utils.h
#pragma once
#include <cstdint>
#include <string>

std::string get_executable_dir();
std::string resolve_path(const std::string& path);
void print_path(const uint_fast32_t* parents, uint_fast32_t start, uint_fast32_t end);