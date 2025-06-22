#include "config/configuration.h"
#include "io/file_io.h"
#include "utils/path_utils.h"
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <string>

#include "utils/random.h"

#ifdef _WIN32
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

// Check if a file exists by trying to stat it
bool file_exists(const char* filename) {
    struct stat buffer{};
    return stat(filename, &buffer) == 0;
}

int main(const int argc, char* argv[]) {
    // Expect exactly one argument: config file path
    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    // Initialize random number generator
    init_random();

    // Print current working directory for debug purposes
    if (char cwd[1024]; GETCWD(cwd, sizeof(cwd))) {
        printf("Current working directory: %s\n", cwd);
    }

    // Resolve the config file path using custom logic
    const std::string resolved = resolve_path(argv[1]);
    printf("Resolved config path: %s\n", resolved.c_str());

    // Initialize file configuration struct with zeros
    File_config file_cfg = {};

    // Read configuration from the resolved file path
    read_config_file(resolved.c_str(), &file_cfg);
    print_config_file(&file_cfg);

    // Initialize main configuration struct with zeros
    Config cfg = {};

    // Simple decision logic for configuration execution
    if (file_cfg.file_name) {
        // Load configuration from file
        run_config_file_load(&file_cfg, &cfg);
    } else if (file_cfg.num_v > 0 && file_cfg.density > 0) {
        // Generate a random graph based on parameters
        run_config_file_var(&file_cfg, &cfg);
    } else {
        fprintf(stderr, "Error: insufficient configuration parameters\n");
        fprintf(stderr, "Either specify file_name or both num_v and density\n");
        return 1;
    }

    // Clean up allocated resources
    free_config(&cfg);
    free_config_file(&file_cfg);

    return 0;
}
