#include "file_io.h"
#include "../config/configuration.h"
#include "../graph/graph.h"
#include "../io/display.h"
#include "../algorithms/sp/dijkstra.h"
#include "../algorithms/sp/bellman_ford.h"
#include "../algorithms/mst/prim.h"
#include "../algorithms/mst/kruskal.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <direct.h>
#include <string>
#include "../utils/timer.h"

#define BREAK_LINE "--------------------------------\n"
#define SECTION_LINE "================================\n"

#include <filesystem>
#include "../utils/path_utils.h"

// Universal function to load a graph from a file
static void load_graph_file(const char *file_name, Config *cfg, bool directed) {
    std::string resolved_path = resolve_path(file_name);
    printf("Loading graph from: %s\n", resolved_path.c_str());

    FILE *file = fopen(resolved_path.c_str(), "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", resolved_path.c_str());
        perror("fopen");
        exit(1);
    }

    U32f num_v, num_e;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fprintf(stderr, "Invalid file format: missing vertex/edge count\n");
        fclose(file);
        exit(1);
    }

    printf("Graph info: vertices=%u, edges=%u\n", num_v, num_e);

    if (num_v == 0) {
        fprintf(stderr, "Error: Graph must have at least 1 vertex\n");
        fclose(file);
        exit(1);
    }

    cfg->num_v = num_v;
    cfg->density = num_e;
    cfg->graph = create_graph(num_v);

    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph structure\n");
        fclose(file);
        exit(1);
    }

    U32f src, dst, weight;
    U32f edge_count = 0;

    // Read edges from file
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Warning: Skipping invalid edge %u->%u (max vertex %u)\n", src, dst, num_v - 1);
            continue;
        }

        add_edge(cfg->graph, src, dst, weight);
        if (!directed) {
            add_edge(cfg->graph, dst, src, weight);
        }
        edge_count++;
    }

    fclose(file);

    printf("Successfully loaded %u edges\n", edge_count);
    if (edge_count != num_e) {
        fprintf(stderr, "Warning: Expected %u edges, loaded %u\n", num_e, edge_count);
    }
}

// Simple graph loader (doesn't touch Config structure)
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
            fprintf(stderr, "Error reading edge %u\n", i);
            continue;
        }

        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Invalid edge: %u -> %u (max vertex %u)\n", src, dst, num_v - 1);
            continue;
        }

        add_edge(*graph, src, dst, weight);
        if (!directed) {
            add_edge(*graph, dst, src, weight);
        }
    }

    fclose(file);
}

// Updated graph loader that stores result into Config
void load_graph_from_file(const char *file_name, Config *cfg, bool directed) {
    std::string resolved_path = resolve_path(file_name);
    printf("Loading graph from: %s\n", resolved_path.c_str());

    FILE *file = fopen(resolved_path.c_str(), "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", resolved_path.c_str());
        perror("fopen");
        exit(1);
    }

    U32f num_v, num_e;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fprintf(stderr, "Invalid file format: missing vertex/edge count\n");
        fclose(file);
        exit(1);
    }

    printf("Graph info: vertices=%u, edges=%u\n", num_v, num_e);

    if (num_v == 0) {
        fprintf(stderr, "Error: Graph must have at least 1 vertex\n");
        fclose(file);
        exit(1);
    }

    cfg->num_v = num_v;
    cfg->density = num_e;
    cfg->graph = create_graph(num_v);

    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph structure\n");
        fclose(file);
        exit(1);
    }

    U32f src, dst, weight;
    U32f edge_count = 0;

    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Warning: Skipping invalid edge %u->%u (max vertex %u)\n", src, dst, num_v - 1);
            continue;
        }

        add_edge(cfg->graph, src, dst, weight);
        if (!directed) {
            add_edge(cfg->graph, dst, src, weight);
        }
        edge_count++;
    }

    fclose(file);

    printf("Successfully loaded %u edges\n", edge_count);
    if (edge_count != num_e) {
        fprintf(stderr, "Warning: Expected %u edges, loaded %u\n", num_e, edge_count);
    }
}

// Loads an undirected graph into Config
static void load_undir_graph(const char *file_name, Config* cfg) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        exit(1);
    }

    U32f num_v, num_e;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fclose(file);
        fprintf(stderr, "Invalid file format\n");
        exit(1);
    }

    cfg->num_v = num_v;
    cfg->density = num_e;
    cfg->graph = create_graph(num_v);

    if (!cfg->graph) {
        fclose(file);
        fprintf(stderr, "Failed to create graph\n");
        exit(1);
    }

    U32f src, dst, weight;
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Invalid vertex index: %u-%u\n", src, dst);
            continue;
        }
        add_edge(cfg->graph, src, dst, weight);
        add_edge(cfg->graph, dst, src, weight);
    }

    fclose(file);
}

// Print graph representations based on config flags
void print_graph_representation(const Config *cfg, const File_config *cfg_file) {
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

    if (cfg_file->out_list && cfg->graph) {
        printf("Adjacency List:\n");
        print_graph(cfg->graph);
        printf(BREAK_LINE);
    }
}

// Run configuration using a randomly generated graph
void run_config_file_var(File_config *cfg_file, Config *cfg) {
    printf("1. Generate random graph and run algorithm\n");
    printf(BREAK_LINE);

    if (cfg_file->num_v == 0 || cfg_file->density <= 0) {
        fprintf(stderr, "Error: Vertex count and density must be specified and non-zero\n");
        return;
    }

    create_config_random_weights(cfg, cfg_file->num_v, cfg_file->density,
                                  cfg_file->alg_type, cfg_file->start_vertex);

    print_graph_representation(cfg, cfg_file);
    free_unused_config(cfg, cfg->alg_type);

    // Select and execute algorithm
    switch (cfg->alg_type) {
        case DIJKSTRA_LIST: dijkstra_list(cfg); break;
        case DIJKSTRA_MATRIX: dijkstra_matrix(cfg); break;
        case BELMAN_FORD_LIST: bellman_ford_list(cfg); break;
        case BELMAN_FORD_MATRIX_EDGE_LIST: bellman_ford_matrix_edge_list(cfg); break;
        case BELMAN_FORD_MATRIX_NO_EDGE_LIST: bellman_ford_matrix_no_edge_list(cfg); break;
        case PRIM_LIST: prim_list(cfg); break;
        case PRIM_MATRIX: prim_matrix(cfg); break;
        case KRUSKAL_LIST: kruskal_list(cfg); break;
        case KRUSKAL_MATRIX: kruskal_matrix(cfg); break;
        default: fprintf(stderr, "Unknown algorithm type\n"); return;
    }

    printf("Results for %s:\n", alg_names[cfg_file->alg_type]);
    printf("Execution time: %.6f ms\n", cfg->execution_time);

    // Output results
    if (cfg->res_sp) {
        printf("Distances:\n");
        for (U32f i = 0; i < cfg->num_v; i++) {
            printf("  to %u: %u (parent: %u)\n", i, cfg->res_sp->distances[i], cfg->res_sp->parents[i]);
        }
    }

    if (cfg->res_prim) {
        printf("MST (Prim):\n");
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (i == cfg->start_vertex) {
                printf("  [root] %u\n", i);
                continue;
            }
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_prim->parent_weight[i].parent,
                   i,
                   cfg->res_prim->parent_weight[i].weight);
        }
    }

    if (cfg->res_kruskal) {
        printf("MST (Kruskal):\n");
        for (U32f i = 0; i < cfg->res_kruskal->num_edges; i++) {
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_kruskal->edges[i].u,
                   cfg->res_kruskal->edges[i].v,
                   cfg->res_kruskal->edges[i].weight);
        }
    }

    printf(SECTION_LINE);
}

// Run configuration using graph loaded from file
void run_config_file_load(File_config *cfg_file, Config *cfg) {
    printf("2. Load graph from file and run algorithm\n");
    printf(BREAK_LINE);

    if (!cfg_file->file_name) {
        fprintf(stderr, "No input file specified\n");
        return;
    }

    bool directed = (
        cfg_file->alg_type == DIJKSTRA_LIST ||
        cfg_file->alg_type == DIJKSTRA_MATRIX ||
        cfg_file->alg_type == BELMAN_FORD_LIST ||
        cfg_file->alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        cfg_file->alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST
    );

    load_graph_from_file(cfg_file->file_name, cfg, directed);

    if (!cfg->graph) {
        printf("Error loading graph from %s\n", cfg_file->file_name);
        return;
    }

    create_config_from_graph(cfg, cfg_file->alg_type, cfg->num_v, cfg->density);
    print_graph_representation(cfg, cfg_file);
    free_unused_config(cfg, cfg->alg_type);

    Timer timer;

    // Select and execute algorithm
    switch (cfg_file->alg_type) {
        case DIJKSTRA_LIST: dijkstra_list(cfg); break;
        case DIJKSTRA_MATRIX: dijkstra_matrix(cfg); break;
        case BELMAN_FORD_LIST: bellman_ford_list(cfg); break;
        case BELMAN_FORD_MATRIX_EDGE_LIST: bellman_ford_matrix_edge_list(cfg); break;
        case BELMAN_FORD_MATRIX_NO_EDGE_LIST: bellman_ford_matrix_no_edge_list(cfg); break;
        case PRIM_LIST: prim_list(cfg); break;
        case PRIM_MATRIX: prim_matrix(cfg); break;
        case KRUSKAL_LIST: kruskal_list(cfg); break;
        case KRUSKAL_MATRIX: kruskal_matrix(cfg); break;
        default: fprintf(stderr, "Unknown algorithm type\n"); return;
    }

    printf("Results for %s:\n", alg_names[cfg_file->alg_type]);
    printf("Execution time: %.6f ms\n", cfg->execution_time);

    if (cfg->res_sp) {
        printf("Distances from vertex %u:\n", cfg->start_vertex);
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (cfg->res_sp->distances[i] == UINT_MAX) {
                printf("  to %u: INFINITY\n", i);
            } else {
                printf("  to %u: %u (parent: %u)\n",
                       i,
                       cfg->res_sp->distances[i],
                       cfg->res_sp->parents[i]);
            }
        }
    }

    if (cfg->res_prim) {
        printf("MST (Prim) edges:\n");
        U32f total_weight = 0;
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (i == cfg->start_vertex) continue;
            if (cfg->res_prim->parent_weight[i].parent != UINT32_MAX) {
                printf("  %u - %u (weight: %u)\n",
                       cfg->res_prim->parent_weight[i].parent,
                       i,
                       cfg->res_prim->parent_weight[i].weight);
                total_weight += cfg->res_prim->parent_weight[i].weight;
            }
        }
        printf("Total MST weight: %u\n", total_weight);
    }

    if (cfg->res_kruskal) {
        printf("MST (Kruskal) edges:\n");
        U32f total_weight = 0;
        for (U32f i = 0; i < cfg->res_kruskal->num_edges; i++) {
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_kruskal->edges[i].u,
                   cfg->res_kruskal->edges[i].v,
                   cfg->res_kruskal->edges[i].weight);
            total_weight += cfg->res_kruskal->edges[i].weight;
        }
        printf("Total MST weight: %u\n", total_weight);
    }

    printf(SECTION_LINE);
}
