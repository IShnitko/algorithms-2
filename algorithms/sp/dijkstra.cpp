#include "../sp/dijkstra.h" // Assuming this contains definitions specific to Dijkstra
#include "../../config/configuration.h"
#include "../../utils/min_heap.h"
#include "../../utils/timer.h"
#include <cstdlib> // For malloc, free, exit
#include <climits> // For UINT_MAX, UINT32_MAX
#include <cstdio>  // For fprintf

// Dijkstra's algorithm implementation for graphs represented by adjacency lists.
void dijkstra_list(Config *cfg) {
    // Validate configuration and graph pointer.
    if (!cfg || !cfg->graph) {
        fprintf(stderr, "Error: Invalid configuration or graph for Dijkstra (adjacency list).\n");
        return;
    }

    Timer timer; // Timer for measuring execution time.
    const U32f n = cfg->graph->num_v; // Number of vertices.
    const U32f start = cfg->start_vertex; // Starting vertex for shortest paths.

    // Validate the start vertex.
    if (start >= n) {
        fprintf(stderr, "Error: Start vertex %lu is out of range [0, %lu] for Dijkstra (adjacency list).\n", start, n - 1);
        return;
    }

    // Allocate memory for results (distances and parents).
    // If cfg->res_sp is already allocated, reuse it; otherwise, allocate.
    if (!cfg->res_sp) {
        cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
        if (cfg->res_sp == nullptr) {
            fprintf(stderr, "Error: Memory allocation failed for res_sp in dijkstra_list.\n");
            return;
        }
    }

    cfg->res_sp->distances = (U32f*)malloc(n * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(n * sizeof(U32f));

    if (cfg->res_sp->distances == nullptr || cfg->res_sp->parents == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for distances or parents in dijkstra_list.\n");
        // Free already allocated memory before exiting
        free(cfg->res_sp->distances);
        free(cfg->res_sp->parents);
        free(cfg->res_sp);
        cfg->res_sp = nullptr; // Set to NULL to indicate failure
        return;
    }

    // Initialize distances: all to infinity (UINT_MAX).
    // Initialize parents: all to a sentinel value (UINT32_MAX) indicating no parent.
    for (U32f i = 0; i < n; ++i) {
        cfg->res_sp->distances[i] = UINT_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start] = 0; // Distance to the start vertex is 0.

    // Create and initialize a min-priority queue (min-heap).
    MinHeap* heap = create_min_heap(n);
    if (heap == nullptr) {
        fprintf(stderr, "Error: Failed to create min heap in dijkstra_list.\n");
        // Free allocated result memory before returning
        free(cfg->res_sp->distances);
        free(cfg->res_sp->parents);
        free(cfg->res_sp);
        cfg->res_sp = nullptr;
        return;
    }
    min_heap_insert(heap, start, 0); // Insert the start vertex with distance 0.

    // Main Dijkstra's algorithm loop: continues until the min-heap is empty.
    while (!min_heap_is_empty(heap)) {
        HeapItem item = min_heap_extract_min(heap); // Extract the vertex with the minimum distance.
        U32f u = item.vertex;

        // If the extracted distance is greater than the current recorded distance,
        // it means we've already found a shorter path to 'u', so skip.
        // This can happen if an item is inserted multiple times with different keys
        // before its final (minimum) key is extracted.
        if (item.key > cfg->res_sp->distances[u]) {
            continue;
        }

        // Iterate through all neighbors of 'u'.
        Node* neighbor = cfg->graph->adjLists[u];
        while (neighbor != nullptr) {
            U32f v = neighbor->vertex;
            U32f weight = neighbor->weight;

            // Check for potential integer overflow before addition:
            // if adding 'weight' to 'distances[u]' would exceed UINT_MAX.
            if (cfg->res_sp->distances[u] > UINT_MAX - weight) {
                // If overflow could occur, it means path is effectively infinite, skip.
                neighbor = neighbor->next;
                continue;
            }

            U32f new_dist = cfg->res_sp->distances[u] + weight; // Calculate new path distance.

            // If a shorter path to 'v' is found.
            if (new_dist < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = new_dist; // Update distance.
                cfg->res_sp->parents[v] = u;         // Set 'u' as the parent of 'v'.

                // Update or insert 'v' in the min-heap.
                if (heap->indices[v] != UINT32_MAX) { // Check if 'v' is already in the heap.
                    min_heap_decrease_key(heap, v, new_dist); // Decrease its key.
                } else {
                    min_heap_insert(heap, v, new_dist); // Insert 'v' into the heap.
                }
            }
            neighbor = neighbor->next;
        }
    }
    cfg->execution_time = timer.elapsed(); // Record the elapsed execution time.

    // Free the dynamically allocated min-heap.
    free_min_heap(heap);
}

// Dijkstra's algorithm implementation for graphs represented by incidence matrices.
// This version uses an array-based approach to find the minimum distance vertex in each step.
void dijkstra_matrix(Config *cfg) {
    // Validate configuration and incidence matrix pointer.
    if (!cfg || !cfg->inc_matrix_dir) {
        fprintf(stderr, "Error: Invalid configuration or incidence matrix for Dijkstra (matrix).\n");
        return;
    }

    Timer timer; // Timer for measuring execution time.
    const U32f n = cfg->num_v; // Number of vertices.
    const U32f m = cfg->density; // Number of edges (columns in the incidence matrix).
    const U32f start = cfg->start_vertex; // Starting vertex.

    // Validate the start vertex.
    if (start >= n) {
        fprintf(stderr, "Error: Start vertex %lu is out of range [0, %lu] for Dijkstra (matrix).\n", start, n-1);
        return;
    }

    // Allocate memory for distances, parents, and visited array.
    U32f* dist = (U32f*)malloc(n * sizeof(U32f));
    U32f* prev = (U32f*)malloc(n * sizeof(U32f));
    bool* visited = (bool*)calloc(n, sizeof(bool)); // calloc initializes to zero (false).

    if (dist == nullptr || prev == nullptr || visited == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for dist, prev, or visited arrays in dijkstra_matrix.\n");
        free(dist); // Free any successfully allocated memory.
        free(prev);
        free(visited);
        return;
    }

    // Initialize distances and parents.
    for (U32f i = 0; i < n; ++i) {
        dist[i] = UINT_MAX;     // Set all distances to infinity.
        prev[i] = UINT32_MAX;   // Set all parents to sentinel.
    }
    dist[start] = 0; // Distance to the start vertex is 0.

    // Main Dijkstra's algorithm loop.
    // The loop runs 'n-1' times to find the shortest paths to 'n-1' vertices.
    for (U32f count = 0; count < n - 1; ++count) {
        // Find the vertex 'u' with the minimum distance among unvisited vertices.
        U32f u = UINT32_MAX;
        U32f min_dist = UINT_MAX;
        for (U32f i = 0; i < n; ++i) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        // If no reachable unvisited vertex is found (e.g., disconnected graph).
        if (u == UINT32_MAX) break;
        visited[u] = true; // Mark 'u' as visited (included in SPT).

        // Update distances to neighbors of 'u'.
        // Iterate through all other vertices 'v'.
        for (U32f v = 0; v < n; ++v) {
            if (visited[v]) continue; // If 'v' is already visited, skip.

            // Search for the edge (u, v) in the incidence matrix.
            // Iterate through all columns (edges) 'e'.
            for (U32f e = 0; e < m; ++e) {
                // In an incidence matrix:
                // inc_matrix_dir[u * m + e] > 0 means 'u' is the destination of edge 'e' (for an incoming edge).
                // inc_matrix_dir[u * m + e] < 0 means 'u' is the source of edge 'e' (for an outgoing edge).
                // So, for an edge from u to v: inc_matrix_dir[u * m + e] < 0 and inc_matrix_dir[v * m + e] > 0
                // Re-evaluating based on typical convention:
                // A positive value at (row u, col e) means edge 'e' enters 'u'.
                // A negative value at (row u, col e) means edge 'e' leaves 'u'.
                // For a directed edge from u to v with weight W:
                // inc_matrix[u * m + e] = -W
                // inc_matrix[v * m + e] = W
                // Other entries in column 'e' are 0.

                // Correct logic for finding an edge from u to v in an incidence matrix:
                // The element at `u * m + e` should be negative (source of the edge)
                // AND the element at `v * m + e` should be positive (destination of the edge).
                if (cfg->inc_matrix_dir[u * m + e] < 0 && cfg->inc_matrix_dir[v * m + e] > 0) {
                    U32f weight = (U32f)(-cfg->inc_matrix_dir[u * m + e]); // Get positive weight.

                    // Check for potential integer overflow.
                    if (dist[u] > UINT_MAX - weight) {
                        continue; // Skip this path if it leads to overflow.
                    }

                    U32f new_dist = dist[u] + weight;

                    // If a shorter path to 'v' is found.
                    if (new_dist < dist[v]) {
                        dist[v] = new_dist; // Update distance.
                        prev[v] = u;        // Set 'u' as parent of 'v'.
                    }
                }
            }
        }
    }
    cfg->execution_time = timer.elapsed(); // Record the elapsed execution time.

    // Store the results in the Config structure.
    // If cfg->res_sp is not allocated, allocate it.
    if (!cfg->res_sp) {
        cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
        if (cfg->res_sp == nullptr) {
            fprintf(stderr, "Error: Memory allocation failed for final res_sp in dijkstra_matrix.\n");
            // Free the temporary arrays to prevent leaks.
            free(dist);
            free(prev);
            free(visited);
            return;
        }
    }
    cfg->res_sp->distances = dist; // Assign the computed distances array.
    cfg->res_sp->parents = prev;   // Assign the computed parents array.

    // Free the temporary visited array.
    free(visited);
}
