#include "config/configuration.h" // Include the main configuration structures and function prototypes.
#include "io/file_io.h"         // Include functions for file input/output related to configuration and graph loading.
#include "utils/path_utils.h"   // Include utility for resolving file paths.
#include <cstring>              // For memset.
#include <cstdio>               // For printf, fprintf, perror.
#include <sys/stat.h>           // For stat (to check file existence).
#include <string>               // For std::string.

#include "utils/random.h"       // Include utility for random number initialization.

// Conditional compilation for getting current working directory.
// _WIN32 is a predefined macro for Microsoft Windows.
#ifdef _WIN32
#include <direct.h> // For _getcwd on Windows.
#define GETCWD _getcwd // Define GETCWD macro for Windows.
#else
#include <unistd.h> // For getcwd on Unix-like systems.
#define GETCWD getcwd // Define GETCWD macro for Unix-like.
#endif

// Function to check if a file exists.
// filename: The path to the file.
// Returns: true if the file exists, false otherwise.
bool file_exists(const char* filename) {
    struct stat buffer; // Structure to hold file status information.
    // stat() attempts to get file status. Returns 0 on success, -1 on failure.
    return (stat(filename, &buffer) == 0);
}

// Main function, entry point of the program.
// argc: Number of command-line arguments.
// argv: Array of command-line argument strings.
int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is provided.
    // Expecting one argument: the configuration file path.
    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1; // Exit with error code 1.
    }

    // Initialize the random number generator. This is crucial for generating random graphs.
    init_random();

    // Debugging information: Print the current working directory.
    char cwd[1024]; // Buffer to store the current working directory.
    if (GETCWD(cwd, sizeof(cwd))) { // GETCWD fills 'cwd' with the current directory path.
        printf("Current working directory: %s\n", cwd);
    }

    // Resolve the path to the configuration file.
    // This handles relative paths and attempts to find the file in common locations.
    std::string resolved = resolve_path(argv[1]);
    printf("Resolved config path: %s\n", resolved.c_str());

    // Create and initialize a File_config structure.
    // memset sets all bytes of the structure to 0, effectively nullifying pointers and setting numeric fields to 0.
    File_config file_cfg;
    memset(&file_cfg, 0, sizeof(File_config));

    // Read configuration parameters from the resolved configuration file.
    read_config_file(resolved.c_str(), &file_cfg);
    // Print the loaded configuration parameters for verification.
    print_config_file(&file_cfg);

    // Create and initialize the main runtime Config structure.
    // This structure will hold the graph data and algorithm results.
    Config cfg;
    memset(&cfg, 0, sizeof(Config));

    // Determine the program's execution mode based on the configuration.
    // If file_name is specified in the config, load graph from file.
    if (file_cfg.file_name) {
        run_config_file_load(&file_cfg, &cfg);
    }
    // If num_v (number of vertices) and density are specified, generate a random graph.
    else if (file_cfg.num_v > 0 && file_cfg.density > 0) {
        run_config_file_var(&file_cfg, &cfg);
    }
    // If neither of the above conditions is met, it's an insufficient configuration.
    else {
        fprintf(stderr, "Error: insufficient configuration parameters\n");
        fprintf(stderr, "Either specify file_name or both num_v and density\n");
        return 1; // Exit with error code 1.
    }

    // Clean up dynamically allocated memory for the main configuration.
    free_config(&cfg);
    // Clean up dynamically allocated memory for the file configuration.
    free_config_file(&file_cfg);

    return 0; // Exit successfully.
}
