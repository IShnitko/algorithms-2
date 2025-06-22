#include "bellman_ford.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/timer.h"
#include <climits>
#include <cstdio>

// Implementation using adjacency list
void bellman_ford_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f start_vertex = cfg->start_vertex;
    Graph* graph = cfg->graph;

    // Initialize result structures
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));

    // Initialize arrays
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Start timer
    Timer timer;
    timer.start();

    // Main algorithm: relax all edges (V-1) times
    bool changed;
    for (U32f i = 1; i < num_v; i++) {
        changed = false;

        for (U32f u = 0; u < num_v; u++) {
            if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

            Node* node = graph->adjLists[u];
            while (node != nullptr) {
                U32f v = node->vertex;
                U32f weight = node->weight;

                // Check for overflow
                if (cfg->res_sp->distances[u] > UINT32_MAX - weight) {
                    node = node->next;
                    continue;
                }

                U32f alt = cfg->res_sp->distances[u] + weight;

                if (alt < cfg->res_sp->distances[v]) {
                    cfg->res_sp->distances[v] = alt;
                    cfg->res_sp->parents[v] = u;
                    changed = true;
                }
                node = node->next;
            }
        }

        // Early exit if no changes occurred
        if (!changed) break;
    }

    // Check for negative weight cycles (shouldn't occur due to non-negative weights)
    bool has_negative_cycle = false;
    for (U32f u = 0; u < num_v; u++) {
        if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

        Node* node = graph->adjLists[u];
        while (node != nullptr) {
            U32f v = node->vertex;
            U32f weight = node->weight;

            if (cfg->res_sp->distances[u] > UINT32_MAX - weight) {
                node = node->next;
                continue;
            }

            U32f alt = cfg->res_sp->distances[u] + weight;

            if (alt < cfg->res_sp->distances[v]) {
                has_negative_cycle = true;
                break;
            }
            node = node->next;
        }
        if (has_negative_cycle) break;
    }

    // Record execution time
    cfg->execution_time = timer.stop();

    if (has_negative_cycle) {
        fprintf(stderr, "Graph contains negative weight cycle\n");
    }
}

// Implementation using incidence matrix with prebuilt edge list
void bellman_ford_matrix_edge_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f density = cfg->density;
    U32f start_vertex = cfg->start_vertex;
    I32f* inc_matrix = cfg->inc_matrix_dir;

    // Initialize result structures
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));

    for (U32f i = 0; i < num_v; i++) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Build edge list from incidence matrix
    struct Edge {
        U32f u, v;
        U32f weight;
    };

    Edge* edges = (Edge*)malloc(density * sizeof(Edge));
    U32f edge_count = 0;

    for (U32f j = 0; j < density; j++) {
        U32f u = UINT32_MAX, v = UINT32_MAX;
        U32f weight = 0;

        for (U32f i = 0; i < num_v; i++) {
            I32f val = inc_matrix[i * density + j];
            if (val < 0) {
                u = i;
                weight = -val; // Weight is always positive
            } else if (val > 0) {
                v = i;
            }
        }

        if (u != UINT32_MAX && v != UINT32_MAX) {
            edges[edge_count].u = u;
            edges[edge_count].v = v;
            edges[edge_count].weight = weight;
            edge_count++;
        }
    }

    // Start timer
    Timer timer;
    timer.start();

    // Main algorithm
    bool changed;
    for (U32f i = 1; i < num_v; i++) {
        changed = false;

        for (U32f j = 0; j < edge_count; j++) {
            U32f u = edges[j].u;
            U32f v = edges[j].v;
            U32f weight = edges[j].weight;

            if (cfg->res_sp->distances[u] == UINT32_MAX) continue;
            if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

            U32f alt = cfg->res_sp->distances[u] + weight;

            if (alt < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = alt;
                cfg->res_sp->parents[v] = u;
                changed = true;
            }
        }

        if (!changed) break;
    }

    // Check for negative weight cycles
    bool has_negative_cycle = false;
    for (U32f j = 0; j < edge_count; j++) {
        U32f u = edges[j].u;
        U32f v = edges[j].v;
        U32f weight = edges[j].weight;

        if (cfg->res_sp->distances[u] == UINT32_MAX) continue;
        if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

        U32f alt = cfg->res_sp->distances[u] + weight;

        if (alt < cfg->res_sp->distances[v]) {
            has_negative_cycle = true;
            break;
        }
    }

    // Record execution time
    cfg->execution_time = timer.stop();
    free(edges);

    if (has_negative_cycle) {
        fprintf(stderr, "Graph contains negative weight cycle\n");
    }
}

// Implementation using incidence matrix without edge list
void bellman_ford_matrix_no_edge_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f density = cfg->density;
    U32f start_vertex = cfg->start_vertex;
    I32f* inc_matrix = cfg->inc_matrix_dir;

    // Initialize result structures
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));

    for (U32f i = 0; i < num_v; i++) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Start timer
    Timer timer;
    timer.start();

    // Main algorithm
    bool changed;
    for (U32f i = 1; i < num_v; i++) {
        changed = false;

        for (U32f j = 0; j < density; j++) {
            U32f u = UINT32_MAX, v = UINT32_MAX;
            U32f weight = 0;

            for (U32f k = 0; k < num_v; k++) {
                I32f val = inc_matrix[k * density + j];
                if (val < 0) {
                    u = k;
                    weight = -val;
                } else if (val > 0) {
                    v = k;
                }
            }

            if (u == UINT32_MAX || v == UINT32_MAX) continue;
            if (cfg->res_sp->distances[u] == UINT32_MAX) continue;
            if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

            U32f alt = cfg->res_sp->distances[u] + weight;

            if (alt < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = alt;
                cfg->res_sp->parents[v] = u;
                changed = true;
            }
        }

        if (!changed) break;
    }

    // Check for negative weight cycles
    bool has_negative_cycle = false;
    for (U32f j = 0; j < density; j++) {
        U32f u = UINT32_MAX, v = UINT32_MAX;
        U32f weight = 0;

        for (U32f k = 0; k < num_v; k++) {
            I32f val = inc_matrix[k * density + j];
            if (val < 0) {
                u = k;
                weight = -val;
            } else if (val > 0) {
                v = k;
            }
        }

        if (u == UINT32_MAX || v == UINT32_MAX) continue;
        if (cfg->res_sp->distances[u] == UINT32_MAX) continue;
        if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

        U32f alt = cfg->res_sp->distances[u] + weight;

        if (alt < cfg->res_sp->distances[v]) {
            has_negative_cycle = true;
            break;
        }
    }

    // Record execution time
    cfg->execution_time = timer.stop();

    if (has_negative_cycle) {
        fprintf(stderr, "Graph contains negative weight cycle\n");
    }
}
