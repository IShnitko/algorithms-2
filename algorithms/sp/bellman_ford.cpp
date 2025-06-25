#include "bellman_ford.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/timer.h"
#include <climits> // For UINT32_MAX
#include <cstdio>
#include <cstdlib> // For malloc, free, exit

// Bellman-Ford algorithm implementation for graphs represented by adjacency lists.
void bellman_ford_list(Config* cfg) {
    U32f num_v = cfg->num_v;             // Number of vertices.
    U32f start_vertex = cfg->start_vertex; // Starting vertex for shortest paths.
    Graph* graph = cfg->graph;           // Pointer to the graph structure.

    // Initialize the shortest path results structure.
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    if (cfg->res_sp == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp in bellman_ford_list.\n");
        exit(EXIT_FAILURE);
    }

    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    if (cfg->res_sp->distances == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp->distances in bellman_ford_list.\n");
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }

    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));
    if (cfg->res_sp->parents == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp->parents in bellman_ford_list.\n");
        free(cfg->res_sp->distances);
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }

    // Initialize distances: all to infinity (UINT32_MAX) except start_vertex.
    // Initialize parents: all to a sentinel value (UINT32_MAX).
    for (U32f i = 0; i < num_v; ++i) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX; // Indicates no parent yet.
    }
    cfg->res_sp->distances[start_vertex] = 0; // Distance to start_vertex is 0.

    // Start timer to measure execution time.
    Timer timer;

    // Main Bellman-Ford algorithm loop: Relax all edges V-1 times.
    // In each iteration, we try to find shorter paths.
    bool changed; // Flag to track if any distance was updated in an iteration.
    for (U32f i = 1; i < num_v; ++i) { // V-1 iterations.
        changed = false;

        // Iterate through all vertices 'u'
        for (U32f u = 0; u < num_v; ++u) {
            // If u is unreachable, skip it.
            if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

            // Iterate through all neighbors 'v' of 'u'.
            Node* node = graph->adjLists[u];
            while (node != nullptr) {
                U32f v = node->vertex;
                U32f weight = node->weight;

                // Check for potential integer overflow before addition.
                // If distances[u] is very large and adding weight would exceed UINT32_MAX.
                if (cfg->res_sp->distances[u] > UINT32_MAX - weight) {
                    node = node->next; // Skip this edge to prevent overflow.
                    continue;
                }

                U32f alt = cfg->res_sp->distances[u] + weight; // Calculate new path distance.

                // If a shorter path to 'v' is found.
                if (alt < cfg->res_sp->distances[v]) {
                    cfg->res_sp->distances[v] = alt; // Update distance.
                    cfg->res_sp->parents[v] = u;     // Update parent.
                    changed = true;                   // Mark that a change occurred.
                }
                node = node->next;
            }
        }

        // If no distances were updated in an entire iteration,
        // it means all shortest paths have been found, so we can break early.
        if (!changed) break;
    }

    // Second pass to detect negative weight cycles.
    // If a distance can still be relaxed after V-1 iterations,
    // a negative cycle reachable from the source exists.
    bool has_negative_cycle = false;
    for (U32f u = 0; u < num_v; ++u) {
        if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

        Node* node = graph->adjLists[u];
        while (node != nullptr) {
            U32f v = node->vertex;
            U32f weight = node->weight;

            // Check for potential integer overflow.
            if (cfg->res_sp->distances[u] > UINT32_MAX - weight) {
                node = node->next;
                continue;
            }

            U32f alt = cfg->res_sp->distances[u] + weight;

            if (alt < cfg->res_sp->distances[v]) {
                has_negative_cycle = true;
                break; // Found a negative cycle.
            }
            node = node->next;
        }
        if (has_negative_cycle) break; // Exit outer loop if cycle found.
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    // Report if a negative cycle was found.
    // Note: The problem statement implies positive weights, so this check might always be false
    // under typical usage, but it's essential for a complete Bellman-Ford implementation.
    if (has_negative_cycle) {
        fprintf(stderr, "Graph contains negative weight cycle\n");
        // In a real application, you might want to adjust distances to a special value
        // like 0 (representing -infinity) for vertices reachable from a negative cycle.
    }
    // No explicit free for cfg->res_sp->distances and cfg->res_sp->parents here
    // as they are part of cfg and assumed to be freed by the caller or global cleanup.
}

// Bellman-Ford algorithm implementation for graphs represented by incidence matrices.
// This version first converts the incidence matrix to an edge list.
void bellman_ford_matrix_edge_list(Config* cfg) {
    U32f num_v = cfg->num_v;             // Number of vertices.
    U32f density = cfg->density;         // Number of edges (columns in incidence matrix).
    U32f start_vertex = cfg->start_vertex; // Starting vertex.
    I32f* inc_matrix = cfg->inc_matrix_dir; // Pointer to the directed incidence matrix.

    // Initialize shortest path results structure.
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    if (cfg->res_sp == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp in bellman_ford_matrix_edge_list.\n");
        exit(EXIT_FAILURE);
    }

    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    if (cfg->res_sp->distances == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp->distances in bellman_ford_matrix_edge_list.\n");
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }

    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));
    if (cfg->res_sp->parents == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp->parents in bellman_ford_matrix_edge_list.\n");
        free(cfg->res_sp->distances);
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }

    // Initialize distances and parents.
    for (U32f i = 0; i < num_v; ++i) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Structure to represent an edge.
    struct Edge {
        U32f u, v;     // Start and end vertices.
        U32f weight;   // Edge weight.
    };

    // Allocate memory for an array of edges. The maximum number of edges is `density`.
    Edge* edges = (Edge*)malloc(density * sizeof(Edge));
    if (edges == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for edges in bellman_ford_matrix_edge_list.\n");
        free(cfg->res_sp->distances);
        free(cfg->res_sp->parents);
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }
    U32f edge_count = 0; // Actual number of edges found.

    // Convert incidence matrix to an edge list.
    // Iterate through each column (representing an edge) of the incidence matrix.
    for (U32f j = 0; j < density; ++j) {
        U32f u = UINT32_MAX, v = UINT32_MAX; // Initialize to sentinel values.
        U32f weight = 0;

        // Find the `u` (source) and `v` (destination) vertices for the current edge `j`.
        for (U32f i = 0; i < num_v; ++i) {
            I32f val = inc_matrix[i * density + j];
            if (val < 0) { // Negative value indicates source vertex.
                u = i;
                weight = (U32f)(-val); // Weight is the absolute value.
            } else if (val > 0) { // Positive value indicates destination vertex.
                v = i;
            }
        }

        // If both source and destination vertices are found, add the edge to the list.
        if (u != UINT32_MAX && v != UINT32_MAX) {
            edges[edge_count].u = u;
            edges[edge_count].v = v;
            edges[edge_count].weight = weight;
            edge_count++;
        }
    }

    // Start timer.
    Timer timer;

    // Main Bellman-Ford algorithm loop: Relax all edges V-1 times.
    bool changed;
    for (U32f i = 1; i < num_v; ++i) { // V-1 iterations.
        changed = false;

        // Iterate through each edge in the constructed edge list.
        for (U32f j = 0; j < edge_count; ++j) {
            U32f u = edges[j].u;
            U32f v = edges[j].v;
            U32f weight = edges[j].weight;

            // If source vertex 'u' is unreachable, skip this edge.
            if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

            // Check for potential integer overflow before addition.
            if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

            U32f alt = cfg->res_sp->distances[u] + weight;

            // If a shorter path to 'v' is found.
            if (alt < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = alt;
                cfg->res_sp->parents[v] = u;
                changed = true;
            }
        }

        // If no distances were updated, break early.
        if (!changed) break;
    }

    // Second pass to detect negative weight cycles.
    bool has_negative_cycle = false;
    for (U32f j = 0; j < edge_count; ++j) {
        U32f u = edges[j].u;
        U32f v = edges[j].v;
        U32f weight = edges[j].weight;

        if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

        // Check for potential integer overflow.
        if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

        U32f alt = cfg->res_sp->distances[u] + weight;

        if (alt < cfg->res_sp->distances[v]) {
            has_negative_cycle = true;
            break;
        }
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    // Free the dynamically allocated edge list.
    free(edges);

    if (has_negative_cycle) {
        fprintf(stderr, "Graph contains negative weight cycle\n");
    }
}

// Bellman-Ford algorithm implementation for graphs represented by incidence matrices.
// This version processes the incidence matrix directly without explicitly building an edge list.
void bellman_ford_matrix_no_edge_list(Config* cfg) {
    U32f num_v = cfg->num_v;             // Number of vertices.
    U32f density = cfg->density;         // Number of edges (columns in incidence matrix).
    U32f start_vertex = cfg->start_vertex; // Starting vertex.
    I32f* inc_matrix = cfg->inc_matrix_dir; // Pointer to the directed incidence matrix.

    // Initialize shortest path results structure.
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    if (cfg->res_sp == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp in bellman_ford_matrix_no_edge_list.\n");
        exit(EXIT_FAILURE);
    }

    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    if (cfg->res_sp->distances == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp->distances in bellman_ford_matrix_no_edge_list.\n");
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }

    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));
    if (cfg->res_sp->parents == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_sp->parents in bellman_ford_matrix_no_edge_list.\n");
        free(cfg->res_sp->distances);
        free(cfg->res_sp);
        exit(EXIT_FAILURE);
    }

    // Initialize distances and parents.
    for (U32f i = 0; i < num_v; ++i) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Start timer.
    Timer timer;

    // Main Bellman-Ford algorithm loop: Relax all edges V-1 times.
    bool changed;
    for (U32f i = 1; i < num_v; ++i) { // V-1 iterations.
        changed = false;

        // Iterate through each edge by iterating through columns of the incidence matrix.
        for (U32f j = 0; j < density; ++j) {
            U32f u = UINT32_MAX, v = UINT32_MAX; // Source and destination vertices for current edge.
            U32f weight = 0;

            // Find the source (u) and destination (v) vertices for the current edge 'j'.
            // An edge 'j' connects two vertices 'k'. One entry is negative (-weight, for source)
            // and one is positive (+weight, for destination).
            for (U32f k = 0; k < num_v; ++k) {
                I32f val = inc_matrix[k * density + j];
                if (val < 0) { // Source vertex.
                    u = k;
                    weight = (U32f)(-val); // Weight is the absolute value.
                } else if (val > 0) { // Destination vertex.
                    v = k;
                }
            }

            // If a valid edge (u, v) wasn't found (e.g., malformed matrix or empty column), skip.
            if (u == UINT32_MAX || v == UINT32_MAX) continue;
            // If source vertex 'u' is unreachable, skip.
            if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

            // Check for potential integer overflow.
            if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

            U32f alt = cfg->res_sp->distances[u] + weight;

            // If a shorter path to 'v' is found.
            if (alt < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = alt;
                cfg->res_sp->parents[v] = u;
                changed = true;
            }
        }

        // If no distances were updated, break early.
        if (!changed) break;
    }

    // Second pass to detect negative weight cycles.
    bool has_negative_cycle = false;
    for (U32f j = 0; j < density; ++j) {
        U32f u = UINT32_MAX, v = UINT32_MAX;
        U32f weight = 0;

        for (U32f k = 0; k < num_v; ++k) {
            I32f val = inc_matrix[k * density + j];
            if (val < 0) {
                u = k;
                weight = (U32f)(-val);
            } else if (val > 0) {
                v = k;
            }
        }

        if (u == UINT32_MAX || v == UINT32_MAX) continue;
        if (cfg->res_sp->distances[u] == UINT32_MAX) continue;

        // Check for potential integer overflow.
        if (cfg->res_sp->distances[u] > UINT32_MAX - weight) continue;

        U32f alt = cfg->res_sp->distances[u] + weight;

        if (alt < cfg->res_sp->distances[v]) {
            has_negative_cycle = true;
            break;
        }
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    if (has_negative_cycle) {
        fprintf(stderr, "Graph contains negative weight cycle\n");
    }
}
