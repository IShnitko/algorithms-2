#include "file_io.h"         // Header for file I/O operations.
#include "../config/configuration.h" // Configuration structures and types.
#include "../graph/graph.     h"     // Graph structure and operations.
#include "../io/display.h"   // Functions for displaying graph representations.
#include "../algorithms/sp/dijkstra.h"     // Dijkstra's algorithm.
#include "../algorithms/sp/bellman_ford.h" // Bellman-Ford algorithm.
#include "../algorithms/mst/prim.h"        // Prim's algorithm.
#include "../algorithms/mst/kruskal.h"     // Kruskal's algorithm.
#include <cstdio>      // For file operations (fopen, fclose, fscanf, fprintf, perror, printf).
#include <cstdlib>     // For exit.
#include <cstring>     // For string manipulation (e.g., strcmp, although not directly used here).
#include <unistd.h>    // For POSIX API (e.g., access, though not directly used here).
#include <string>      // For std::string.
#include "../utils/timer.h"    // For Timer utility to measure execution time.
#include <climits>     // For UINT_MAX (maximum value for unsigned int).
#include <cstdint>     // For UINT32_MAX (maximum value for uint32_t).
#include <inttypes.h>  // For SCNuFAST32 (macro for scanning uint_fast32_t).

// Define constant strings for visual separation in output.
#define BREAK_LINE "--------------------------------\n"
#define SECTION_LINE "================================\n"

// Standard C++ filesystem (if used, it's typically for path manipulation).
#include <filesystem>
#include "../utils/path_utils.h" // Custom utility for path resolution.

// Universal function to load a graph from a file.
// file_name: The path to the graph file.
// cfg: Pointer to the Config structure where graph data will be stored.
// directed: Boolean flag; true if the graph should be treated as directed, false for undirected.
static void load_graph_file(const char *file_name, Config *cfg, bool directed) {
    // Resolve the file path to handle relative paths correctly.
    std::string resolved_path = resolve_path(file_name);
    printf("Loading graph from: %s\n", resolved_path.c_str());

    FILE *file = fopen(resolved_path.c_str(), "r");
    // Check if the file was opened successfully.
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", resolved_path.c_str());
        perror("fopen"); // Print system error message.
        exit(1);         // Exit on critical error.
    }

    U32f num_v, num_e; // Variables to store number of vertices and edges from the file.
    // Read the first line of the file, which should contain num_v and num_e.
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fprintf(stderr, "Invalid file format: missing vertex/edge count\n");
        fclose(file);
        exit(1);
    }

    printf("Graph info: vertices=%lu, edges=%lu\n", num_v, num_e);

    // Validate the number of vertices.
    if (num_v == 0) {
        fprintf(stderr, "Error: Graph must have at least 1 vertex\n");
        fclose(file);
        exit(1);
    }

    cfg->num_v = num_v;     // Set number of vertices in config.
    cfg->density = num_e;   // Set number of edges (density) in config.
    cfg->graph = create_graph(num_v); // Create the graph structure.

    // Check if graph creation was successful.
    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph structure\n");
        fclose(file);
        exit(1);
    }

    U32f src, dst, weight; // Variables for source, destination, and weight of an edge.
    U32f edge_count = 0;   // Counter for edges actually loaded.

    // Read edges from the file until EOF or invalid format.
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        // Validate vertex indices of the edge.
        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Warning: Skipping invalid edge %lu->%lu (max vertex %lu)\n",
                    src, dst, num_v - 1);
            continue; // Skip to the next edge.
        }

        add_edge(cfg->graph, src, dst, weight); // Add the edge.
        if (!directed) {
            add_edge(cfg->graph, dst, src, weight); // Add reverse edge for undirected graphs.
        }
        edge_count++;
    }

    fclose(file); // Close the graph file.

    printf("Successfully loaded %lu edges\n", edge_count);
    // Warn if the number of loaded edges does not match the expected count from the file header.
    if (edge_count != num_e) {
        fprintf(stderr, "Warning: Expected %lu edges, loaded %lu\n", num_e, edge_count);
    }
}

// A more specific helper to load a simple graph (used internally if needed).
// graph: Double pointer to a Graph structure (so it can be allocated and assigned).
static void load_simple_graph(const char *file_name, Graph** graph, bool directed) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        perror("fopen failed");
        exit(1);
    }

    U32f num_v, num_e;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fclose(file);
        fprintf(stderr, "Invalid graph file format\n");
        exit(1);
    }

    *graph = create_graph(num_v);
    if (!*graph) {
        fclose(file);
        fprintf(stderr, "Graph creation failed\n");
        exit(1);
    }

    U32f src, dst, weight;
    for (U32f i = 0; i < num_e; i++) {
        if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) != 3) {
            fprintf(stderr, "Error reading edge %lu\n", i);
            continue;
        }

        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Invalid edge: %lu -> %lu (max vertex %lu)\n", src, dst, num_v - 1);
            continue;
        }

        add_edge(*graph, src, dst, weight);
        if (!directed) {
            add_edge(*graph, dst, src, weight);
        }
    }

    fclose(file);
}

// Public function to load a graph from a file, similar to load_graph_file but with slightly
// different parameter handling and potentially for external use.
// file_name: The path to the graph file.
// cfg: Pointer to the Config structure where graph data will be stored.
// directed: Boolean flag; true if the graph should be treated as directed, false for undirected.
void load_graph_from_file(const char *file_name, Config *cfg, bool directed) {
    // Resolve the file path.
    std::string resolved_path = resolve_path(file_name);
    printf("Loading graph from: %s\n", resolved_path.c_str());

    FILE *file = fopen(resolved_path.c_str(), "r");
    // Check if the file was opened successfully.
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", resolved_path.c_str());
        perror("fopen");
        exit(1);
    }

    U32f num_v_file, num_e_file; // Variables for num_v and num_e read from file
    // Read the first line of the file for vertex and edge counts.
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v_file, &num_e_file) != 2) {
        fprintf(stderr, "Invalid file format: missing vertex/edge count\n");
        fclose(file);
        exit(1);
    }

    printf("Graph info: vertices=%lu, edges=%lu\n", num_v_file, num_e_file);

    // Validate the number of vertices.
    if (num_v_file == 0) {
        fprintf(stderr, "Error: Graph must have at least 1 vertex\n");
        fclose(file);
        exit(1);
    }

    // Assign file-read counts to config.
    cfg->num_v = num_v_file;
    cfg->density = num_e_file;
    cfg->graph = create_graph(num_v_file); // Create the graph structure.

    // Check if graph creation was successful.
    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph structure\n");
        fclose(file);
        exit(1);
    }

    U32f src, dst, weight;
    U32f edge_count = 0;

    // Read and add edges from the file.
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        // Validate vertex indices.
        if (src >= num_v_file || dst >= num_v_file) {
            fprintf(stderr, "Warning: Skipping invalid edge %lu->%lu (max vertex %lu)\n",
                    src, dst, num_v_file - 1);
            continue;
        }

        add_edge(cfg->graph, src, dst, weight); // Add directed edge.
        if (!directed) {
            add_edge(cfg->graph, dst, src, weight); // Add reverse edge for undirected.
        }
        edge_count++;
    }

    fclose(file); // Close the file.

    printf("Successfully loaded %lu edges\n", edge_count);
    // Warn if the loaded edge count mismatches the expected count.
    if (edge_count != num_e_file) {
        fprintf(stderr, "Warning: Expected %lu edges, loaded %lu\n", num_e_file, edge_count);
    }
}

// Static helper to load an undirected graph specifically (might be redundant with load_graph_from_file).
static void load_undir_graph(const char *file_name, Config* cfg) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        exit(1);
    }

    U32f num_v_file, num_e_file;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v_file, &num_e_file) != 2) {
        fclose(file);
        fprintf(stderr, "Invalid file format\n");
        exit(1);
    }

    cfg->num_v = num_v_file;
    cfg->density = num_e_file;
    cfg->graph = create_graph(num_v_file);

    if (!cfg->graph) {
        fclose(file);
        fprintf(stderr, "Failed to create graph\n");
        exit(1);
    }

    U32f src, dst, weight;
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        if (src >= num_v_file || dst >= num_v_file) {
            fprintf(stderr, "Invalid vertex index: %lu-%lu\n", src, dst);
            continue;
        }
        add_edge(cfg->graph, src, dst, weight); // Add edge.
        add_edge(cfg->graph, dst, src, weight); // Add reverse edge for undirected.
    }

    fclose(file);
}

// Prints the graph representations (adjacency list and/or incidence matrix) based on config flags.
// cfg: Pointer to the runtime Config structure.
// cfg_file: Pointer to the File_config structure (contains output flags).
void print_graph_representation(const Config *cfg, const File_config *cfg_file) {
    // Print incidence matrix if requested.
    if (cfg_file->out_matrix) {
        if (cfg->inc_matrix_dir) {
            printf("Directed Incidence Matrix:\n");
            print_inc_dir_matrix(cfg->inc_matrix_dir, cfg->num_v, cfg->density);
            printf(BREAK_LINE);
        }
        else if (cfg->inc_matrix_undir) {
            printf("Undirected Incidence Matrix:\n");
            print_inc_undir_matrix(cfg->inc_matrix_undir, cfg->num_v, cfg->density);
            printf(BREAK_LINE);
        }
    }

    // Print adjacency list if requested and graph exists.
    if (cfg_file->out_list && cfg->graph) {
        printf("Adjacency List:\n");
        print_graph(cfg->graph);
        printf(BREAK_LINE);
    }
}

// Runs an algorithm with a randomly generated graph based on parameters from File_config.
// cfg_file: Parameters read from the config file (for graph generation).
// cfg: Runtime Config structure to be populated and used for algorithm execution.
void run_config_file_var(File_config *cfg_file, Config *cfg) {
    printf("1. Generate random graph and run algorithm\n");
    printf(BREAK_LINE);

    // Validate essential parameters for graph generation.
    if (cfg_file->num_v == 0) {
        fprintf(stderr, "Error: Vertex count (num_v) must be specified and greater than 0.\n");
        return;
    }
    if (cfg_file->density <= 0) { // Density here is likely a ratio or target edge count.
        fprintf(stderr, "Error: Density must be specified and >0.\n");
        return;
    }

    // Create the graph configuration with random weights.
    create_config_random_weights(cfg, cfg_file->num_v, cfg_file->density,
                                cfg_file->alg_type, cfg_file->start_vertex);

    // Free unused graph representations (e.g., if a matrix algorithm is chosen, free adjacency list).
    free_unused_config(cfg, cfg->alg_type);

    // Execute the chosen algorithm based on cfg->alg_type.
    switch (cfg->alg_type) {
        case DIJKSTRA_LIST:
            dijkstra_list(cfg);
            break;
        case DIJKSTRA_MATRIX:
            dijkstra_matrix(cfg);
            break;
        case BELMAN_FORD_LIST:
            bellman_ford_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_EDGE_LIST:
            bellman_ford_matrix_edge_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_NO_EDGE_LIST:
            bellman_ford_matrix_no_edge_list(cfg);
            break;
        case PRIM_LIST:
            prim_list(cfg);
            break;
        case PRIM_MATRIX:
            prim_matrix(cfg);
            break;
        case KRUSKAL_LIST:
            kruskal_list(cfg);
            break;
        case KRUSKAL_MATRIX:
            kruskal_matrix(cfg);
            break;
        default:
            fprintf(stderr, "Unknown algorithm type: %s\n", alg_names[cfg_file->alg_type]);
            break;
    }

    printf("Results for %s:\n", alg_names[cfg_file->alg_type]);
    printf("Execution time: %.6f ms\n", cfg->execution_time); // Print measured execution time.

    printf(SECTION_LINE); // End section marker.
}

// Runs an algorithm using a graph loaded from a file specified in File_config.
// cfg_file: Parameters read from the config file (including file_name).
// cfg: Runtime Config structure to be populated and used for algorithm execution.
void run_config_file_load(File_config *cfg_file, Config *cfg) {
    printf("2. Load graph from file and run algorithm\n");
    printf(BREAK_LINE);

    // Validate if an input file name is provided.
    if (!cfg_file->file_name) {
        fprintf(stderr, "No input file specified\n");
        return;
    }

    // Determine if the graph should be treated as directed based on the algorithm type.
    bool directed = (
        cfg_file->alg_type == DIJKSTRA_LIST ||
        cfg_file->alg_type == DIJKSTRA_MATRIX ||
        cfg_file->alg_type == BELMAN_FORD_LIST ||
        cfg_file->alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        cfg_file->alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST
    );

    // Load the graph from the specified file.
    load_graph_from_file(cfg_file->file_name, cfg, directed);

    // Check if graph loading was successful.
    if (!cfg->graph) {
        printf("Error loading graph from %s\n", cfg_file->file_name);
        return;
    }

    // Create necessary matrix representations from the loaded graph, if required by the algorithm.
    // The `create_config_from_graph` function effectively transforms the graph representation
    // as needed by the selected algorithm type.
    create_config_from_graph(cfg, cfg_file->alg_type, cfg->num_v, cfg->density);

    // Print graph representations (adjacency list and/or incidence matrix) if requested.
    print_graph_representation(cfg, cfg_file);

    // Free unused graph representations after matrix conversion (if any).
    free_unused_config(cfg, cfg->alg_type);

    // Execute the chosen algorithm.
    switch (cfg_file->alg_type) {
        case DIJKSTRA_LIST:
            dijkstra_list(cfg);
            break;
        case DIJKSTRA_MATRIX:
            dijkstra_matrix(cfg);
            break;
        case BELMAN_FORD_LIST:
            bellman_ford_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_EDGE_LIST:
            bellman_ford_matrix_edge_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_NO_EDGE_LIST:
            bellman_ford_matrix_no_edge_list(cfg);
            break;
        case PRIM_LIST:
            prim_list(cfg);
            break;
        case PRIM_MATRIX:
            prim_matrix(cfg);
            break;
        case KRUSKAL_LIST:
            kruskal_list(cfg);
            break;
        case KRUSKAL_MATRIX:
            kruskal_matrix(cfg);
            break;
        default:
            fprintf(stderr, "Unknown algorithm type: %s\n", alg_names[cfg_file->alg_type]);
            break;
    }

    // Print results.
    printf("Results for %s:\n", alg_names[cfg_file->alg_type]);
    printf("Execution time: %.6f ms\n", cfg->execution_time);

    // Print shortest path results if available (for SSSP algorithms).
    if (cfg->res_sp) {
        printf("Distances from vertex %lu:\n", cfg->start_vertex);
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (cfg->res_sp->distances[i] == UINT32_MAX) { // Check for "infinity" distance.
                printf("  to %lu: INFINITY\n", i);
            } else {
                printf("  to %lu: %lu (parent: %lu)\n",
                       i,
                       cfg->res_sp->distances[i],
                       cfg->res_sp->parents[i]);
            }
        }
    }

    // Print MST results for Prim's algorithm if available.
    if (cfg->res_prim) {
        printf("MST (Prim) edges:\n");
        U32f total_weight = 0;
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (i == cfg->start_vertex) continue; // Skip the start vertex itself.
            // Check if a valid parent exists (not the sentinel value).
            if (cfg->res_prim->parent_weight[i].parent != UINT32_MAX) {
                printf("  %lu - %lu (weight: %lu)\n",
                       cfg->res_prim->parent_weight[i].parent,
                       i,
                       cfg->res_prim->parent_weight[i].weight);
                total_weight += cfg->res_prim->parent_weight[i].weight;
            }
        }
        printf("Total MST weight: %lu\n", total_weight);
    }

    // Print MST results for Kruskal's algorithm if available.
    if (cfg->res_kruskal) {
        printf("MST (Kruskal) edges:\n");
        U32f total_weight = 0;
        for (U32f i = 0; i < cfg->res_kruskal->num_edges; i++) {
            printf("  %lu - %lu (weight: %lu)\n",
                   cfg->res_kruskal->edges[i].u,
                   cfg->res_kruskal->edges[i].v,
                   cfg->res_kruskal->edges[i].weight);
            total_weight += cfg->res_kruskal->edges[i].weight;
        }
        printf("Total MST weight: %lu\n", total_weight);
    }

    printf(SECTION_LINE); // End section marker.
}
