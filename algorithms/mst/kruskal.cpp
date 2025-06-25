#include "kruskal.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/dsu.h"
#include "../../utils/timer.h"
#include <cstdlib>
#include <cstdio>

// Structure to represent an edge for Kruskal's algorithm.
struct Edge {
    U32f u;      // Start vertex of the edge.
    U32f v;      // End vertex of the edge.
    U32f weight; // Weight of the edge.
};

// Comparison function for sorting edges by weight.
// Used with qsort.
int compare_edges(const void* a, const void* b) {
    const Edge* edgeA = (const Edge*)a;
    const Edge* edgeB = (const Edge*)b;
    if (edgeA->weight < edgeB->weight) return -1;
    if (edgeA->weight > edgeB->weight) return 1;
    return 0;
}

// Kruskal's algorithm implementation for graphs represented by adjacency lists.
void kruskal_list(Config* cfg) {
    U32f num_v = cfg->num_v;     // Number of vertices.
    Graph* graph = cfg->graph;   // Pointer to the graph structure.

    // Calculate the total number of unique edges in the undirected graph.
    U32f total_edges = 0;
    for (U32f i = 0; i < num_v; ++i) {
        Node* node = graph->adjLists[i];
        while (node != nullptr) {
            // For an undirected graph, count each edge only once (e.g., (u,v) but not (v,u)).
            if (i < node->vertex) {
                total_edges++;
            }
            node = node->next;
        }
    }

    // Allocate memory for all edges.
    Edge* edges = (Edge*)malloc(total_edges * sizeof(Edge));
    if (edges == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for edges in kruskal_list.\n");
        exit(EXIT_FAILURE);
    }
    U32f edge_count = 0;

    // Populate the array with all edges.
    for (U32f i = 0; i < num_v; ++i) {
        Node* node = graph->adjLists[i];
        while (node != nullptr) {
            U32f j = node->vertex;
            // Add each edge only once to avoid duplicates (e.g., if (u,v) is added, don't add (v,u)).
            if (i < j) {
                edges[edge_count].u = i;
                edges[edge_count].v = j;
                edges[edge_count].weight = node->weight;
                edge_count++;
            }
            node = node->next;
        }
    }

    // Sort all edges in non-decreasing order of their weights.
    qsort(edges, total_edges, sizeof(Edge), compare_edges);

    // Initialize Disjoint Set Union (DSU) structure for `num_v` vertices.
    DSU dsu(num_v);

    // Initialize the result structure for Kruskal's algorithm.
    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    if (cfg->res_kruskal == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_kruskal in kruskal_list.\n");
        free(edges);
        exit(EXIT_FAILURE);
    }

    if (num_v == 0) {
        fprintf(stderr, "Error: num_v must be greater than 0 for Kruskal's algorithm.\n");
        free(edges);
        free(cfg->res_kruskal);
        exit(EXIT_FAILURE);
    }
    // A Minimum Spanning Tree (MST) of V vertices has exactly V-1 edges.
    cfg->res_kruskal->edges = static_cast<KruskalEdge*>(malloc((num_v - 1) * sizeof(KruskalEdge)));
    if (cfg->res_kruskal->edges == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_kruskal->edges in kruskal_list.\n");
        free(edges);
        free(cfg->res_kruskal);
        exit(EXIT_FAILURE);
    }
    cfg->res_kruskal->num_edges = 0;

    // Start timer to measure execution time.
    Timer timer;

    // Main Kruskal's algorithm loop: Iterate through sorted edges.
    for (U32f i = 0; i < total_edges; ++i) {
        U32f u = edges[i].u;
        U32f v = edges[i].v;

        // If including this edge does not form a cycle (i.e., u and v are in different sets).
        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v); // Union the sets of u and v.

            // Add the edge to the result MST.
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].u = u;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].v = v;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].weight = edges[i].weight;
            cfg->res_kruskal->num_edges++;

            // If V-1 edges have been added, the MST is complete.
            if (cfg->res_kruskal->num_edges == num_v - 1) break;
        }
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    // Free dynamically allocated memory for edges.
    free(edges);
}

// Kruskal's algorithm implementation for graphs represented by incidence matrices.
void kruskal_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;           // Number of vertices.
    U32f density = cfg->density;       // Represents the number of edges for incidence matrix.
    U32f* inc_matrix = cfg->inc_matrix_undir; // Pointer to the undirected incidence matrix.

    // Allocate memory for all edges.
    Edge* edges = (Edge*)malloc(density * sizeof(Edge));
    if (edges == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for edges in kruskal_matrix.\n");
        exit(EXIT_FAILURE);
    }
    U32f edge_count = 0;

    // Populate the array with edges by iterating through columns (representing edges)
    // of the incidence matrix.
    for (U32f j = 0; j < density; ++j) {
        U32f u = UINT32_MAX; // Initialize with a sentinel value.
        U32f v = UINT32_MAX; // Initialize with a sentinel value.
        U32f weight = 0;

        // Find the two vertices connected by the current edge (column j).
        for (U32f i = 0; i < num_v; ++i) {
            if (inc_matrix[i * density + j] != 0) { // If there's a connection.
                if (u == UINT32_MAX) { // First vertex found for this edge.
                    u = i;
                    weight = inc_matrix[i * density + j]; // Weight is stored in the matrix entry.
                } else { // Second vertex found for this edge.
                    v = i;
                    break; // Found both vertices, move to the next edge.
                }
            }
        }

        // If both vertices were found (i.e., a valid edge).
        if (u != UINT32_MAX && v != UINT32_MAX) {
            edges[edge_count].u = u;
            edges[edge_count].v = v;
            edges[edge_count].weight = weight;
            edge_count++;
        }
    }

    // Sort all edges in non-decreasing order of their weights.
    qsort(edges, edge_count, sizeof(Edge), compare_edges);

    // Initialize Disjoint Set Union (DSU) structure for `num_v` vertices.
    DSU dsu(num_v);

    // Initialize the result structure for Kruskal's algorithm.
    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    if (cfg->res_kruskal == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_kruskal in kruskal_matrix.\n");
        free(edges);
        exit(EXIT_FAILURE);
    }
    // A Minimum Spanning Tree (MST) of V vertices has exactly V-1 edges.
    cfg->res_kruskal->edges = (KruskalEdge*)malloc((num_v - 1) * sizeof(KruskalEdge));
    if (cfg->res_kruskal->edges == nullptr) {
        fprintf(stderr, "Error: Memory allocation failed for res_kruskal->edges in kruskal_matrix.\n");
        free(edges);
        free(cfg->res_kruskal);
        exit(EXIT_FAILURE);
    }
    cfg->res_kruskal->num_edges = 0;

    // Start timer to measure execution time.
    Timer timer;

    // Main Kruskal's algorithm loop: Iterate through sorted edges.
    for (U32f i = 0; i < edge_count; ++i) {
        U32f u = edges[i].u;
        U32f v = edges[i].v;

        // If including this edge does not form a cycle.
        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v); // Union the sets of u and v.

            // Add the edge to the result MST.
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].u = u;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].v = v;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].weight = edges[i].weight;
            cfg->res_kruskal->num_edges++;

            // If V-1 edges have been added, the MST is complete.
            if (cfg->res_kruskal->num_edges == num_v - 1) break;
        }
    }

    // Record the elapsed execution time.
    cfg->execution_time = timer.elapsed();

    // Free dynamically allocated memory for edges.
    free(edges);
}
