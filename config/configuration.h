#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <inttypes.h> // For fixed-width integer types like uint_fast32_t, int_fast32_t
#include <stdio.h>    // For FILE, printf, etc. (though not directly used here, often included)
#include <stdlib.h>   // For malloc, free, exit (though not directly used here, often included)
#include <string.h>   // For memset, strcmp, strdup, strchr (though not directly used here, often included)
#include "../graph/graph.h" // Include for Graph structure definition

// Common type definitions for faster integer operations.
typedef uint_fast32_t U32f; // Unsigned 32-bit fast integer.
typedef int_fast32_t I32f;  // Signed 32-bit fast integer.

// Enumeration for different algorithm types.
// Used to categorize and select which algorithm to run.
enum Alg_type {
    DIJKSTRA_LIST,                     // Dijkstra's algorithm using adjacency list.
    DIJKSTRA_MATRIX,                   // Dijkstra's algorithm using incidence matrix.
    BELMAN_FORD_LIST,                  // Bellman-Ford algorithm using adjacency list.
    BELMAN_FORD_MATRIX_EDGE_LIST,      // Bellman-Ford using incidence matrix, iterating edges.
    BELMAN_FORD_MATRIX_NO_EDGE_LIST,   // Bellman-Ford using incidence matrix, no explicit edge list.
    PRIM_LIST,                         // Prim's algorithm using adjacency list.
    PRIM_MATRIX,                       // Prim's algorithm using incidence matrix.
    KRUSKAL_LIST,                      // Kruskal's algorithm using adjacency list.
    KRUSKAL_MATRIX,                    // Kruskal's algorithm using incidence matrix.
    ALG_TYPE_COUNT                     // Total number of algorithm types.
};

// External declaration of an array that holds string names for each algorithm type.
// This allows printing the algorithm name based on its enum value.
extern const char* alg_names[ALG_TYPE_COUNT];

// Structure to store results for Single Source Shortest Path (SSSP) algorithms (e.g., Dijkstra, Bellman-Ford).
// This structure must be defined before the Config structure, as Config contains a pointer to it.
typedef struct Res_sp {
    U32f* distances; // Array to store the shortest distance from the source to each vertex.
    U32f* parents;   // Array to store the predecessor vertex in the shortest path from the source.
} Res_sp;

// Structure to store information about a parent and its associated edge weight
// for algorithms like Prim's (Minimum Spanning Tree).
typedef struct PrimResult {
    U32f parent; // The parent vertex in the MST.
    U32f weight; // The weight of the edge connecting to the parent.
} PrimResult;

// Structure to represent an edge for Kruskal's algorithm (MST).
typedef struct KruskalEdge {
    U32f u;      // First vertex of the edge.
    U32f v;      // Second vertex of the edge.
    U32f weight; // Weight of the edge.
} KruskalEdge;

// Structure to store results for Prim's algorithm.
typedef struct Res_prim {
    PrimResult* parent_weight; // Array of PrimResult, one for each vertex (except source).
} Res_prim;

// Structure to store results for Kruskal's algorithm.
typedef struct Res_kruskal {
    KruskalEdge* edges;   // Array of edges forming the MST.
    U32f num_edges;       // Number of edges in the MST.
} Res_kruskal;

// Main configuration structure for graph algorithms.
// Contains all necessary data for graph generation, representation, and algorithm execution results.
typedef struct Config {
    Graph* graph;              // Pointer to the adjacency list representation of the graph.
    I32f* inc_matrix_dir;      // Pointer to the directed incidence matrix.
    U32f* inc_matrix_undir;    // Pointer to the undirected incidence matrix.
    U32f num_v;                // Number of vertices in the graph.
    U32f density;              // For generated graphs, this can be the actual number of edges.
    U32f start_vertex;         // The starting vertex for SSSP algorithms.
    Res_sp* res_sp;            // Pointer to the SSSP results (distances, parents).
    Res_prim* res_prim;        // Pointer to Prim's algorithm results.
    Res_kruskal* res_kruskal;  // Pointer to Kruskal's algorithm results.
    enum Alg_type alg_type;    // The type of algorithm to be executed.
    double execution_time;     // Field to store the execution time of the algorithm.
} Config;

// Configuration structure for reading parameters from a file.
// This separates file-specific parameters from the runtime Config structure.
typedef struct File_config {
    char* file_name;    // Name of the input graph file (if loading from file).
    U32f num_v;         // Number of vertices (for graph generation).
    double density;     // Density of the graph (for graph generation).
    U32f start_vertex;  // Starting vertex for SSSP algorithms.
    int out_matrix;     // Flag: true if matrix output is desired.
    int out_list;       // Flag: true if adjacency list output is desired.
    enum Alg_type alg_type; // The algorithm type specified in the file.
} File_config;

// Function prototypes for configuration management.

// Initializes a Config structure for a randomly generated graph.
void create_config_random_weights(Config *cfg, U32f num_v, double density,
                                 enum Alg_type alg_type, U32f start_vertex);

// Initializes a Config structure using an already existing graph (e.g., loaded from file).
void create_config_from_graph(Config *cfg, enum Alg_type alg_type,
                             U32f num_v, U32f density);

// Frees graph representations (e.g., adjacency list) that are not needed by the chosen algorithm,
// especially if a matrix representation is used.
void free_unused_config(Config *cfg, enum Alg_type alg_type);

// Frees all dynamically allocated memory within a Config structure.
void free_config(Config *cfg);

// Frees dynamically allocated memory specific to the File_config structure (e.g., file_name string).
void free_config_file(File_config *cfg);

// Reads configuration parameters from a specified file into a File_config structure.
void read_config_file(const char* filename, File_config* config);

// Prints the contents of a File_config structure to standard output.
void print_config_file(File_config *cfg_file);

// Function to run algorithms based on variable parameters from a file (e.g., generated graphs).
void run_config_file_var(File_config *cfg_file, Config *cfg);

// Function to run algorithms based on loading a graph from a file.
void run_config_file_load(File_config *cfg_file, Config *cfg);

#endif // CONFIGURATION_H