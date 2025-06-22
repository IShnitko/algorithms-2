#include "prim.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/min_heap.h"
#include "../../utils/timer.h"
#include <climits>
#include <cstdlib>
#include <cstdio>

// Prim's algorithm using adjacency list representation
void prim_list(Config* cfg) {
    U32f num_v = cfg->num_v;                   // Number of vertices
    U32f start_vertex = cfg->start_vertex;     // Starting vertex
    Graph* graph = cfg->graph;                 // Pointer to graph

    // Allocate memory for result structure
    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    cfg->res_prim->parent_weight = (PrimResult*)malloc(num_v * sizeof(PrimResult));

    // Allocate helper arrays
    U32f* key = (U32f*)malloc(num_v * sizeof(U32f));       // Minimum edge weight to each vertex
    U32f* parent = (U32f*)malloc(num_v * sizeof(U32f));    // Parent of each vertex in MST
    bool* inMST = (bool*)malloc(num_v * sizeof(bool));     // Inclusion status in MST

    // Initialize all vertices
    for (U32f i = 0; i < num_v; i++) {
        key[i] = UINT32_MAX;       // Set key to "infinity"
        parent[i] = UINT32_MAX;    // No parent yet
        inMST[i] = false;          // Not in MST
    }
    key[start_vertex] = 0;                     // Start vertex has key 0
    parent[start_vertex] = start_vertex;       // It is its own parent initially

    // Create min-heap and insert the start vertex
    MinHeap* heap = create_min_heap(num_v);
    min_heap_insert(heap, start_vertex, key[start_vertex]);

    // Start timer
    Timer timer;
    timer.start();

    // Main loop of Prim's algorithm
    while (!min_heap_is_empty(heap)) {
        HeapItem min_item = min_heap_extract_min(heap); // Extract vertex with minimum key
        U32f u = min_item.vertex;
        inMST[u] = true;                                // Mark as included in MST

        // Traverse all adjacent vertices
        Node* node = graph->adjLists[u];
        while (node != nullptr) {
            U32f v = node->vertex;
            U32f weight = node->weight;

            // Update key and parent if better edge is found
            if (!inMST[v] && weight < key[v]) {
                key[v] = weight;
                parent[v] = u;

                // Update or insert in heap
                if (heap->indices[v] != UINT32_MAX) {
                    min_heap_decrease_key(heap, v, key[v]);
                } else {
                    min_heap_insert(heap, v, key[v]);
                }
            }
            node = node->next; // Move to next adjacent vertex
        }
    }

    // Stop timer and store execution time
    cfg->execution_time = timer.stop();

    // Save MST results
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_prim->parent_weight[i].parent = parent[i];
        cfg->res_prim->parent_weight[i].weight = (i == start_vertex) ? 0 : key[i];
    }

    // Free allocated memory
    free_min_heap(heap);
    free(key);
    free(parent);
    free(inMST);
}

// Prim's algorithm using incidence matrix representation
void prim_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;                   // Number of vertices
    U32f density = cfg->density;               // Number of edges
    U32f start_vertex = cfg->start_vertex;     // Starting vertex
    U32f* inc_matrix = cfg->inc_matrix_undir;  // Pointer to incidence matrix

    // Allocate memory for result structure
    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    cfg->res_prim->parent_weight = (PrimResult*)malloc(num_v * sizeof(PrimResult));

    // Allocate helper arrays
    U32f* key = (U32f*)malloc(num_v * sizeof(U32f));       // Minimum edge weight to each vertex
    U32f* parent = (U32f*)malloc(num_v * sizeof(U32f));    // Parent of each vertex in MST
    bool* inMST = (bool*)malloc(num_v * sizeof(bool));     // Inclusion status in MST

    // Initialize all vertices
    for (U32f i = 0; i < num_v; i++) {
        key[i] = UINT32_MAX;
        parent[i] = UINT32_MAX;
        inMST[i] = false;
    }
    key[start_vertex] = 0;
    parent[start_vertex] = start_vertex;

    // Create min-heap and insert the start vertex
    MinHeap* heap = create_min_heap(num_v);
    min_heap_insert(heap, start_vertex, key[start_vertex]);

    // Start timer
    Timer timer;
    timer.start();

    // Main loop of Prim's algorithm
    while (!min_heap_is_empty(heap)) {
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;
        inMST[u] = true;

        // Iterate over edges connected to vertex u
        for (U32f j = 0; j < density; j++) {
            U32f idx = u * density + j;
            if (inc_matrix[idx] != 0) {
                // Find the other vertex connected by edge j
                U32f v = UINT32_MAX;
                for (U32f k = 0; k < num_v; k++) {
                    if (k == u) continue;
                    if (inc_matrix[k * density + j] != 0) {
                        v = k;
                        break;
                    }
                }

                if (v == UINT32_MAX) continue; // Invalid edge
                U32f weight = inc_matrix[idx];

                // Update key and parent if better edge is found
                if (!inMST[v] && weight < key[v]) {
                    key[v] = weight;
                    parent[v] = u;

                    // Update or insert in heap
                    if (heap->indices[v] != UINT32_MAX) {
                        min_heap_decrease_key(heap, v, key[v]);
                    } else {
                        min_heap_insert(heap, v, key[v]);
                    }
                }
            }
        }
    }

    // Stop timer and store execution time
    cfg->execution_time = timer.stop();

    // Save MST results
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_prim->parent_weight[i].parent = parent[i];
        cfg->res_prim->parent_weight[i].weight = (i == start_vertex) ? 0 : key[i];
    }

    // Free allocated memory
    free_min_heap(heap);
    free(key);
    free(parent);
    free(inMST);
}
