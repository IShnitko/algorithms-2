#include "prim.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/min_heap.h"
#include "../../utils/timer.h"
#include <climits> // For UINT32_MAX
#include <cstdlib>
#include <cstdio>

// Prim's algorithm implementation for graphs represented by adjacency lists.
void prim_list(Config* cfg) {
    U32f num_v = cfg->num_v;             // Number of vertices.
    U32f start_vertex = cfg->start_vertex; // Starting vertex for Prim's algorithm.
    Graph* graph = cfg->graph;           // Pointer to the graph structure.

    // Initialize the result structure for Prim's algorithm.
    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    if (cfg->res_prim == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_prim in prim_list.\n");
        exit(EXIT_FAILURE);
    }
    cfg->res_prim->parent_weight = (PrimResult*)malloc(num_v * sizeof(PrimResult));
    if (cfg->res_prim->parent_weight == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_prim->parent_weight in prim_list.\n");
        free(cfg->res_prim);
        exit(EXIT_FAILURE);
    }

    // Initialize arrays for Prim's algorithm:
    // key[i]: Stores the minimum edge weight to connect vertex i to the MST.
    // parent[i]: Stores the parent of vertex i in the MST.
    // inMST[i]: A boolean flag indicating if vertex i is already included in the MST.
    U32f* key = (U32f*)malloc(num_v * sizeof(U32f));
    U32f* parent = (U32f*)malloc(num_v * sizeof(U32f));
    bool* inMST = (bool*)malloc(num_v * sizeof(bool));

    if (key == nullptr || parent == nullptr || inMST == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for key, parent, or inMST arrays in prim_list.\n");
        free(cfg->res_prim->parent_weight);
        free(cfg->res_prim);
        free(key);
        free(parent);
        free(inMST);
        exit(EXIT_FAILURE);
    }

    // Initialize all key values to infinity (UINT32_MAX), parent to a sentinel, and inMST to false.
    for (U32f i = 0; i < num_v; ++i) {
        key[i] = UINT32_MAX;
        parent[i] = UINT32_MAX; // Sentinel value indicating no parent.
        inMST[i] = false;
    }

    // Set the key of the start_vertex to 0 and its parent to itself (as it's the root of the MST).
    key[start_vertex] = 0;
    parent[start_vertex] = start_vertex;

    // Create and initialize a min-priority queue (min-heap).
    MinHeap* heap = create_min_heap(num_v);
    if (heap == nullptr) {
        fprintf(stderr, "Error: Failed to create min heap in prim_list.\n");
        free(cfg->res_prim->parent_weight);
        free(cfg->res_prim);
        free(key);
        free(parent);
        free(inMST);
        exit(EXIT_FAILURE);
    }
    // Insert the start_vertex into the min-heap with its key.
    min_heap_insert(heap, start_vertex, key[start_vertex]);

    // Start timer to measure execution time.
    Timer timer;

    // Main Prim's algorithm loop: continues until the min-heap is empty.
    while (!min_heap_is_empty(heap)) {
        // Extract the vertex with the minimum key value from the heap.
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;
        inMST[u] = true; // Mark the extracted vertex as included in MST.

        // Iterate through all adjacent vertices of u.
        Node* node = graph->adjLists[u];
        while (node != nullptr) {
            U32f v = node->vertex;
            U32f weight = node->weight;

            // If vertex v is not yet in MST and the edge (u,v) offers a smaller weight
            // than the current key[v] (i.e., a better way to connect v to the MST).
            if (!inMST[v] && weight < key[v]) {
                key[v] = weight; // Update key[v] with the new smaller weight.
                parent[v] = u;   // Set u as the parent of v in the MST.

                // If v is already in the heap, update its key. Otherwise, insert it.
                if (heap->indices[v] != UINT32_MAX) { // Check if v is present in the heap.
                    min_heap_decrease_key(heap, v, key[v]);
                } else {
                    min_heap_insert(heap, v, key[v]);
                }
            }
            node = node->next;
        }
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    // Store the results (parent and edge weight) for each vertex.
    for (U32f i = 0; i < num_v; ++i) {
        cfg->res_prim->parent_weight[i].parent = parent[i];
        // The weight for the start_vertex is 0, for others it's their key value.
        cfg->res_prim->parent_weight[i].weight = (i == start_vertex) ? 0 : key[i];
    }

    // Free dynamically allocated resources.
    free_min_heap(heap);
    free(key);
    free(parent);
    free(inMST);
}

// Prim's algorithm implementation for graphs represented by incidence matrices.
void prim_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;               // Number of vertices.
    U32f density = cfg->density;           // Represents the number of edges (columns) in the incidence matrix.
    U32f start_vertex = cfg->start_vertex; // Starting vertex for Prim's algorithm.
    U32f* inc_matrix = cfg->inc_matrix_undir; // Pointer to the undirected incidence matrix.

    // Initialize the result structure for Prim's algorithm.
    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    if (cfg->res_prim == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_prim in prim_matrix.\n");
        exit(EXIT_FAILURE);
    }
    cfg->res_prim->parent_weight = (PrimResult*)malloc(num_v * sizeof(PrimResult));
    if (cfg->res_prim->parent_weight == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_prim->parent_weight in prim_matrix.\n");
        free(cfg->res_prim);
        exit(EXIT_FAILURE);
    }

    // Initialize arrays for Prim's algorithm (key, parent, inMST).
    U32f* key = (U32f*)malloc(num_v * sizeof(U32f));
    U32f* parent = (U32f*)malloc(num_v * sizeof(U32f));
    bool* inMST = (bool*)malloc(num_v * sizeof(bool));

    if (key == nullptr || parent == nullptr || inMST == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for key, parent, or inMST arrays in prim_matrix.\n");
        free(cfg->res_prim->parent_weight);
        free(cfg->res_prim);
        free(key);
        free(parent);
        free(inMST);
        exit(EXIT_FAILURE);
    }

    // Initialize all key values to infinity (UINT32_MAX), parent to a sentinel, and inMST to false.
    for (U32f i = 0; i < num_v; ++i) {
        key[i] = UINT32_MAX;
        parent[i] = UINT32_MAX; // Sentinel value indicating no parent.
        inMST[i] = false;
    }

    // Set the key of the start_vertex to 0 and its parent to itself.
    key[start_vertex] = 0;
    parent[start_vertex] = start_vertex;

    // Create and initialize a min-priority queue (min-heap).
    MinHeap* heap = create_min_heap(num_v);
    if (heap == nullptr) {
        fprintf(stderr, "Error: Failed to create min heap in prim_matrix.\n");
        free(cfg->res_prim->parent_weight);
        free(cfg->res_prim);
        free(key);
        free(parent);
        free(inMST);
        exit(EXIT_FAILURE);
    }
    // Insert the start_vertex into the min-heap.
    min_heap_insert(heap, start_vertex, key[start_vertex]);

    // Start timer to measure execution time.
    Timer timer;

    // Main Prim's algorithm loop.
    while (!min_heap_is_empty(heap)) {
        // Extract the vertex with the minimum key value.
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;
        inMST[u] = true; // Mark as included in MST.

        // Iterate through all edges (columns `j`) in the incidence matrix.
        for (U32f j = 0; j < density; ++j) {
            // Calculate the index for vertex `u` in the current edge `j`.
            U32f idx_u_j = u * density + j;

            // If vertex `u` is incident to edge `j`.
            if (inc_matrix[idx_u_j] != 0) {
                // Find the other vertex `v` connected by edge `j`.
                U32f v = UINT32_MAX;
                for (U32f k = 0; k < num_v; ++k) {
                    // Ensure it's not `u` itself and `k` is incident to edge `j`.
                    if (k != u && inc_matrix[k * density + j] != 0) {
                        v = k;
                        break; // Found the other vertex.
                    }
                }

                // If a valid second vertex `v` was not found, continue to the next edge.
                if (v == UINT32_MAX) continue;

                // The weight of the edge (u,v) is stored in the incidence matrix at u's entry for edge j.
                U32f weight = inc_matrix[idx_u_j];

                // If vertex v is not yet in MST and the current edge weight is less than key[v].
                if (!inMST[v] && weight < key[v]) {
                    key[v] = weight; // Update key[v].
                    parent[v] = u;   // Set u as parent of v.

                    // Update or insert v in the min-heap.
                    if (heap->indices[v] != UINT32_MAX) { // Check if v is present in the heap.
                        min_heap_decrease_key(heap, v, key[v]);
                    } else {
                        min_heap_insert(heap, v, key[v]);
                    }
                }
            }
        }
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    // Store the results (parent and edge weight) for each vertex.
    for (U32f i = 0; i < num_v; ++i) {
        cfg->res_prim->parent_weight[i].parent = parent[i];
        cfg->res_prim->parent_weight[i].weight = (i == start_vertex) ? 0 : key[i];
    }

    // Free dynamically allocated resources.
    free_min_heap(heap);
    free(key);
    free(parent);
    free(inMST);
}