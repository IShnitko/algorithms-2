#ifndef FILE_IO_H
#define FILE_IO_H

#include "../config/configuration.h"

void load_graph_from_file(const char *file_name, File_config *cfg_file, Config* cfg);
void print_graph_representation(const Config *cfg, const File_config *cfg_file);
void run_config_file_var(File_config *cfg_file, Config *cfg);
void run_config_file_load(File_config *cfg_file, Config *cfg);

#endif // FILE_IO_H