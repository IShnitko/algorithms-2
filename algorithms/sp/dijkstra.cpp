#include "../../config/configuration.h"
#include "../../utils/min_heap.h"
#include <cstdlib>
#include <climits>
#include <cstdio>

/**
 * Runs Dijkstra algorithm using adjacency list representation and a min-heap.
 * Stores results (distances and parents) in cfg->res_sp.
 */
void dijkstra_list(Config *cfg) {
    if (!cfg || !cfg->graph) {
        fprintf(stderr, "Invalid configuration for Dijkstra\n");
        return;
    }

    const U32f n = cfg->graph->num_v;
    const U32f start = cfg->start_vertex;

    // Check that start vertex is within valid range
    if (start >= n) {
        fprintf(stderr, "Start vertex %u is out of range [0, %u]\n", start, n - 1);
        return;
    }

    // Allocate memory for results if not allocated yet
    if (!cfg->res_sp) {
        cfg->res_sp = (Res_sp *)malloc(sizeof(Res_sp));
        if (!cfg->res_sp) {
            fprintf(stderr, "Memory allocation failed for result structure\n");
            return;
        }
        cfg->res_sp->distances = (U32f *)malloc(n * sizeof(U32f));
        cfg->res_sp->parents = (U32f *)malloc(n * sizeof(U32f));
        if (!cfg->res_sp->distances || !cfg->res_sp->parents) {
            fprintf(stderr, "Memory allocation failed for distances or parents arrays\n");
            free(cfg->res_sp->distances);
            free(cfg->res_sp->parents);
            free(cfg->res_sp);
            cfg->res_sp = NULL;
            return;
        }
    }

    // Initialize distances to max and parents to "undefined"
    for (U32f i = 0; i < n; i++) {
        cfg->res_sp->distances[i] = UINT_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX; // UINT32_MAX marks "undefined"
    }
    cfg->res_sp->distances[start] = 0;

    // Create a min-heap and insert the start vertex with distance 0
    MinHeap *heap = create_min_heap(n);
    min_heap_insert(heap, start, 0);

    // Main loop: extract min distance vertex and relax edges
    while (!min_heap_is_empty(heap)) {
        HeapItem item = min_heap_extract_min(heap);
        U32f u = item.vertex;

        // Iterate over neighbors of u
        Node *neighbor = cfg->graph->adjLists[u];
        while (neighbor != NULL) {
            U32f v = neighbor->vertex;
            U32f weight = neighbor->weight;

            // Prevent overflow in distance addition
            if (cfg->res_sp->distances[u] > UINT_MAX - weight) {
                neighbor = neighbor->next;
                continue;
            }

            U32f new_dist = cfg->res_sp->distances[u] + weight;

            // Relax edge if new shorter path is found
            if (new_dist < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = new_dist;
                cfg->res_sp->parents[v] = u;

                // Update or insert vertex in heap
                if (heap->indices[v] != UINT32_MAX) {
                    min_heap_decrease_key(heap, v, new_dist);
                } else {
                    min_heap_insert(heap, v, new_dist);
                }
            }

            neighbor = neighbor->next;
        }
    }

    // Free min-heap memory
    free_min_heap(heap);
}

/**
 * Runs Dijkstra algorithm using an incidence matrix representation (directed graph).
 * Stores results (distances and parents) in cfg->res_sp.
 */
void dijkstra_matrix(Config *cfg) {
    if (!cfg || !cfg->inc_matrix_dir) return;

    const U32f n = cfg->num_v;
    const U32f m = cfg->density;    // Number of edges per vertex (matrix width)
    const U32f start = cfg->start_vertex;

    // Allocate memory for distance, parent and visited arrays
    U32f *dist = (U32f *)malloc(n * sizeof(U32f));
    U32f *prev = (U32f *)malloc(n * sizeof(U32f));
    bool *visited = (bool *)calloc(n, sizeof(bool));
    if (!dist || !prev || !visited) {
        fprintf(stderr, "Memory allocation failed in dijkstra_matrix\n");
        free(dist);
        free(prev);
        free(visited);
        return;
    }

    // Initialize distances and parents
    for (U32f i = 0; i < n; i++) {
        dist[i] = UINT_MAX;
        prev[i] = UINT32_MAX;
    }
    dist[start] = 0;

    // Main loop: find closest unvisited vertex and relax edges
    for (U32f count = 0; count < n - 1; count++) {
        U32f u = UINT32_MAX;
        U32f min_dist = UINT_MAX;

        // Find vertex with minimum distance among unvisited
        for (U32f i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        if (u == UINT32_MAX) break; // No reachable vertex left
        visited[u] = true;

        // Relax edges from u to all other vertices v
        for (U32f v = 0; v < n; v++) {
            if (visited[v]) continue;

            // Search for edge u->v in incidence matrix
            for (U32f e = 0; e < m; e++) {
                if (cfg->inc_matrix_dir[u * m + e] > 0 && // outgoing edge from u
                    cfg->inc_matrix_dir[v * m + e] < 0) { // incoming edge to v

                    U32f weight = cfg->inc_matrix_dir[u * m + e];
                    U32f new_dist = dist[u] + weight;

                    // Relax edge if shorter path found
                    if (new_dist < dist[v]) {
                        dist[v] = new_dist;
                        prev[v] = u;
                    }
                }
            }
        }
    }

    // Store results in config struct
    if (!cfg->res_sp) {
        cfg->res_sp = (Res_sp *)malloc(sizeof(Res_sp));
        if (!cfg->res_sp) {
            fprintf(stderr, "Memory allocation failed for result structure\n");
            free(dist);
            free(prev);
            free(visited);
            return;
        }
    }
    cfg->res_sp->distances = dist;
    cfg->res_sp->parents = prev;

    free(visited);
}
