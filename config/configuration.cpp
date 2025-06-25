#include "configuration.h"
#include "../graph/generators.h"
#include "../io/display.h"
#include <cstdlib>  // For malloc, free, exit
#include <cstring>  // For memset, strcmp, strdup, strchr
#include <cstdio>   // For FILE, fopen, fprintf, sscanf, fgets, fclose, printf
#include <string>   // For std::string (though not directly used with C-style strings)
// #include <pstl/parallel_backend_utils.h> // This header seems unusual for a C project; typically for C++ Parallel STL. Keeping as-is.
#include "../utils/path_utils.h" // Utility for path operations.
#include "../utils/random.h"     // Include for init_random (as mentioned in original comment).

#define MAX_WEIGHT 4096 // Maximum weight for generated edges.

// Array of algorithm names, mapping Alg_type enum values to string representations.
const char* alg_names[ALG_TYPE_COUNT] = {
    "dijkstra_list",
    "dijkstra_matrix",
    "belman_ford_list",
    "belman_ford_matrix_edge_list",
    "belman_ford_matrix_no_edge_list",
    "prim_list",
    "prim_matrix",
    "kruskal_list",
    "kruskal_matrix"
};

// Reads configuration parameters from a specified file into a File_config structure.
// file_name: The path to the configuration file.
// cfg: Pointer to the File_config structure to populate.
void read_config_file(const char* file_name, File_config* cfg) {
    FILE* file = fopen(file_name, "r");
    // Check if the file was opened successfully.
    if (!file) {
        fprintf(stderr, "Cannot open config file: %s\n", file_name);
        return; // Return without modifying cfg if file cannot be opened.
    }

    // Initialize the File_config structure to all zeros.
    // This ensures all members have a default/known state before parsing.
    memset(cfg, 0, sizeof(File_config));
    // cfg->alg_type is implicitly initialized to 0 (UNKNOWN_ALG) by memset.

    char line[256]; // Buffer to read each line from the file.
    // Read the file line by line until EOF or an error occurs.
    while (fgets(line, sizeof(line), file)) {
        // Skip comment lines (starting with '#') and empty lines.
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char key[128], value[128]; // Buffers for key and value extracted from a line.
        // Attempt to parse the line into a key and a value.
        // Expects format "key value".
        if (sscanf(line, "%127s %127s", key, value) != 2) continue; // If parsing fails, skip line.

        // Remove potential carriage return ('\r') and newline ('\n') characters
        // from the 'value' string, ensuring clean string comparisons.
        char* cr = strchr(value, '\r');
        if (cr) *cr = '\0'; // Null-terminate at '\r'.
        cr = strchr(value, '\n');
        if (cr) *cr = '\0'; // Null-terminate at '\n'.

        // Compare 'key' with known configuration parameters and set corresponding values in 'cfg'.
        if (strcmp(key, ".alg_type") == 0) {
            // Map string algorithm names to their respective enum values.
            if (strcmp(value, "dijkstra_list") == 0) cfg->alg_type = DIJKSTRA_LIST;
            else if (strcmp(value, "dijkstra_matrix") == 0) cfg->alg_type = DIJKSTRA_MATRIX;
            else if (strcmp(value, "belman_ford_list") == 0) cfg->alg_type = BELMAN_FORD_LIST;
            else if (strcmp(value, "belman_ford_matrix_edge_list") == 0) cfg->alg_type = BELMAN_FORD_MATRIX_EDGE_LIST;
            else if (strcmp(value, "belman_ford_matrix_no_edge_list") == 0) cfg->alg_type = BELMAN_FORD_MATRIX_NO_EDGE_LIST;
            else if (strcmp(value, "prim_list") == 0) cfg->alg_type = PRIM_LIST;
            else if (strcmp(value, "prim_matrix") == 0) cfg->alg_type = PRIM_MATRIX;
            else if (strcmp(value, "kruskal_list") == 0) cfg->alg_type = KRUSKAL_LIST;
            else if (strcmp(value, "kruskal_matrix") == 0) cfg->alg_type = KRUSKAL_MATRIX;
            else fprintf(stderr, "Unknown algorithm type: %s\n", value); // Warn for unknown types.
        }
        else if (strcmp(key, ".file_name") == 0) {
            // Duplicate the file name string.
            cfg->file_name = strdup(value);
            // Handle potential memory allocation failure for strdup.
            if (cfg->file_name == NULL) {
                fprintf(stderr, "Memory allocation failed for file_name in read_config_file.\n");
                // In a more robust application, might exit or free other resources.
            }
        }
        else if (strcmp(key, ".start_vertex") == 0) {
            cfg->start_vertex = (U32f)atoi(value); // Convert string to unsigned integer.
        }
        else if (strcmp(key, ".out_list") == 0) {
            cfg->out_list = (strcmp(value, "true") == 0); // Set boolean based on "true" string.
        }
        else if (strcmp(key, ".out_matrix") == 0) {
            cfg->out_matrix = (strcmp(value, "true") == 0); // Set boolean based on "true" string.
        }
        else if (strcmp(key, ".num_v") == 0) {
            cfg->num_v = (U32f)atoi(value); // Convert string to unsigned integer.
        }
        else if (strcmp(key, ".density") == 0) {
            cfg->density = atof(value); // Convert string to double.
        }
    }
    fclose(file); // Close the configuration file.
}

// Prints the current configuration loaded from a file to standard output.
// cfg: Pointer to the File_config structure to print.
void print_config_file(File_config *cfg) {
    printf("==== CONFIGURATION ====\n");
    // Print algorithm name from the `alg_names` array using the enum value as index.
    printf("Algorithm: %s\n", alg_names[cfg->alg_type]);
    printf("Vertices: %u\n", cfg->num_v);
    printf("Density: %.2f\n", cfg->density);
    printf("Start vertex: %u\n", cfg->start_vertex);
    // Print boolean values as "true" or "false".
    printf("Output matrix: %s\n", cfg->out_matrix ? "true" : "false");
    printf("Output list: %s\n", cfg->out_list ? "true" : "false");
    // Print file name, or "none" if not specified.
    printf("Input file: %s\n", cfg->file_name ? cfg->file_name : "none");
    printf("========================\n\n");
}

// Calculates the target number of edges for a directed graph based on density and number of vertices.
// density: Graph density (0.0 to 1.0).
// num_v: Number of vertices.
// Returns: The calculated number of edges.
static U32f density_dir(double density, U32f num_v) {
    // For a directed graph, maximum edges = V * (V - 1).
    return (U32f)(density * num_v * (num_v - 1));
}

// Calculates the target number of edges for an undirected graph based on density and number of vertices.
// density: Graph density (0.0 to 1.0).
// num_v: Number of vertices.
// Returns: The calculated number of edges.
static U32f density_undir(double density, U32f num_v) {
    // For an undirected graph, maximum edges = V * (V - 1) / 2.
    return (U32f)(density * num_v * (num_v - 1) / 2);
}

// Creates a configuration for a randomly generated graph with random edge weights.
// cfg: Pointer to the Config structure to populate.
// num_v: Number of vertices in the generated graph.
// density: Density of the generated graph (0.0 to 1.0).
// alg_type: The type of algorithm that will use this graph (determines directed/undirected).
// start_vertex: The starting vertex for algorithms like Dijkstra or Bellman-Ford.
void create_config_random_weights(Config *cfg, U32f num_v, double density,
                                 enum Alg_type alg_type, U32f start_vertex) {
    // Input validation for parameters.
    if (num_v == 0) {
        fprintf(stderr, "Error: Vertex count must be greater than 0.\n");
        exit(EXIT_FAILURE); // Exit on critical error.
    }
    if (density < 0 || density > 1) {
        fprintf(stderr, "Error: Density must be in [0, 1] range.\n");
        exit(EXIT_FAILURE); // Exit on critical error.
    }
    if (start_vertex >= num_v) {
        fprintf(stderr, "Error: Start vertex must be less than vertex count.\n");
        exit(EXIT_FAILURE); // Exit on critical error.
    }

    // Initialize the Config structure to all zeros.
    memset(cfg, 0, sizeof(Config));
    cfg->num_v = num_v;
    cfg->start_vertex = start_vertex;
    cfg->alg_type = alg_type;

    // Determine if the graph should be directed based on the algorithm type.
    bool directed = (
        alg_type == DIJKSTRA_LIST ||
        alg_type == DIJKSTRA_MATRIX ||
        alg_type == BELMAN_FORD_LIST ||
        alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST
    );

    // Calculate the target number of edges based on graph type (directed/undirected).
    U32f target_edges = directed ? density_dir(density, num_v) : density_undir(density, num_v);
    cfg->density = target_edges; // Store the actual number of edges as density in Config.

    // Create the graph structure (adjacency list).
    cfg->graph = create_graph(num_v);
    // Handle potential memory allocation failure for the graph.
    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph structure\n");
        exit(EXIT_FAILURE);
    }

    // Generate the graph (add vertices and edges) and set random weights.
    if (directed) {
        create_rand_dir_graph(cfg->graph, target_edges, start_vertex);
        set_rand_weights_dir(cfg->graph, 1, MAX_WEIGHT); // Weights between 1 and MAX_WEIGHT.
    } else {
        create_rand_undir_graph(cfg->graph, target_edges);
        set_rand_weights_undir(cfg->graph, 1, MAX_WEIGHT); // Weights between 1 and MAX_WEIGHT.
    }

    // Create matrix representations if required by the algorithm type.
    if (alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        cfg->inc_matrix_dir = create_inc_dir_matrix(cfg->graph, target_edges);
        // Handle potential memory allocation failure for incidence matrix.
        if (cfg->inc_matrix_dir == NULL) {
            fprintf(stderr, "Memory allocation failed for inc_matrix_dir.\n");
            // Free previously allocated graph if matrix allocation fails.
            free_graph(cfg->graph);
            cfg->graph = NULL;
            exit(EXIT_FAILURE);
        }
    }
    else if (alg_type == PRIM_MATRIX || alg_type == KRUSKAL_MATRIX) {
        cfg->inc_matrix_undir = create_inc_undir_matrix(cfg->graph, target_edges);
        // Handle potential memory allocation failure for incidence matrix.
        if (cfg->inc_matrix_undir == NULL) {
            fprintf(stderr, "Memory allocation failed for inc_matrix_undir.\n");
            free_graph(cfg->graph);
            cfg->graph = NULL;
            exit(EXIT_FAILURE);
        }
    }
}

// Creates a configuration using an existing graph (presumably loaded from file).
// This function primarily focuses on setting algorithm type and creating matrix representations
// if needed, assuming cfg->graph is already populated.
// cfg: Pointer to the Config structure. It's assumed cfg->graph is already set.
// alg_type: The type of algorithm to be run.
// num_v: Number of vertices in the existing graph.
// density: Number of edges in the existing graph.
void create_config_from_graph(Config *cfg, enum Alg_type alg_type,
                             U32f num_v, U32f density) {
    // Set algorithm type, number of vertices, and actual edge count (density) in cfg.
    cfg->alg_type = alg_type;
    cfg->num_v = num_v;
    cfg->density = density;

    // Create matrix representations based on the algorithm type, if they are required.
    // The matrix generation functions use the already existing cfg->graph.
    if (alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        cfg->inc_matrix_dir = create_inc_dir_matrix(cfg->graph, density);
        // Handle potential memory allocation failure.
        if (cfg->inc_matrix_dir == NULL) {
            fprintf(stderr, "Memory allocation failed for inc_matrix_dir in create_config_from_graph.\n");
            // Depending on context, might need to free cfg->graph or exit.
        }
    }
    else if (alg_type == PRIM_MATRIX || alg_type == KRUSKAL_MATRIX) {
        cfg->inc_matrix_undir = create_inc_undir_matrix(cfg->graph, density);
        // Handle potential memory allocation failure.
        if (cfg->inc_matrix_undir == NULL) {
            fprintf(stderr, "Memory allocation failed for inc_matrix_undir in create_config_from_graph.\n");
        }
    }
}

// Frees graph resources if the chosen algorithm type does not require the adjacency list representation.
// This is an optimization to save memory when a matrix representation is used instead.
// cfg: Pointer to the Config structure.
// alg_type: The type of algorithm to be run.
void free_unused_config(Config *cfg, enum Alg_type alg_type) {
    // Determine if the algorithm is matrix-based.
    bool is_matrix_alg = (
        alg_type == DIJKSTRA_MATRIX ||
        alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST ||
        alg_type == PRIM_MATRIX ||
        alg_type == KRUSKAL_MATRIX
    );

    // If it's a matrix algorithm AND the graph (adjacency list) exists, free it.
    if (is_matrix_alg && cfg->graph) {
        free_graph(cfg->graph);
        cfg->graph = nullptr; // Set pointer to NULL after freeing.
    }
}

// Frees all dynamically allocated memory within the Config structure.
// This function should be called when the Config object is no longer needed.
// cfg: Pointer to the Config structure to free.
void free_config(Config *cfg) {
    if (!cfg) return; // Do nothing if cfg is NULL.

    // Free the graph (adjacency list) if it exists.
    if (cfg->graph) {
        free_graph(cfg->graph);
        cfg->graph = nullptr;
    }

    // Free the directed incidence matrix if it exists.
    if (cfg->inc_matrix_dir) {
        free(cfg->inc_matrix_dir);
        cfg->inc_matrix_dir = nullptr;
    }

    // Free the undirected incidence matrix if it exists.
    if (cfg->inc_matrix_undir) {
        free(cfg->inc_matrix_undir);
        cfg->inc_matrix_undir = nullptr;
    }

    // Free shortest path results (distances and parents) if they exist.
    if (cfg->res_sp) {
        if (cfg->res_sp->distances) {
            free(cfg->res_sp->distances);
            cfg->res_sp->distances = nullptr;
        }
        if (cfg->res_sp->parents) {
            free(cfg->res_sp->parents);
            cfg->res_sp->parents = nullptr;
        }
        free(cfg->res_sp);
        cfg->res_sp = nullptr;
    }

    // Free Prim's algorithm results if they exist.
    if (cfg->res_prim) {
        if (cfg->res_prim->parent_weight) {
            free(cfg->res_prim->parent_weight);
            cfg->res_prim->parent_weight = nullptr;
        }
        free(cfg->res_prim);
        cfg->res_prim = nullptr;
    }

    // Free Kruskal's algorithm results if they exist.
    if (cfg->res_kruskal) {
        if (cfg->res_kruskal->edges) {
            free(cfg->res_kruskal->edges);
            cfg->res_kruskal->edges = nullptr;
        }
        free(cfg->res_kruskal);
        cfg->res_kruskal = nullptr;
    }
}

// Frees dynamically allocated memory within the File_config structure.
// Specifically, frees the file_name string if it was duplicated using strdup.
// cfg: Pointer to the File_config structure to free.
void free_config_file(File_config* cfg) {
    if (!cfg) return; // Do nothing if cfg is NULL.

    if (cfg->file_name) {
        free(cfg->file_name);
        cfg->file_name = nullptr; // Set pointer to NULL after freeing.
    }
}
