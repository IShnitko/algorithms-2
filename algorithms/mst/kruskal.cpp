#include "kruskal.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/dsu.h"
#include "../../utils/timer.h"
#include <cstdlib>
#include <cstdio>

// Edge structure for Kruskal's algorithm
struct Edge {
    U32f u;
    U32f v;
    U32f weight;
};

// Comparator function for qsort: sorts edges by weight (ascending)
int compare_edges(const void* a, const void* b) {
    Edge* edgeA = (Edge*)a;
    Edge* edgeB = (Edge*)b;
    if (edgeA->weight < edgeB->weight) return -1;
    if (edgeA->weight > edgeB->weight) return 1;
    return 0;
}

// Kruskal's algorithm using adjacency list representation
void kruskal_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    Graph* graph = cfg->graph;

    // Count total number of edges (for undirected graph count only i < j)
    U32f total_edges = 0;
    for (U32f i = 0; i < num_v; i++) {
        Node* node = graph->adjLists[i];
        while (node != nullptr) {
            if (i < node->vertex) {
                total_edges++;
            }
            node = node->next;
        }
    }

    // Allocate memory for all edges
    Edge* edges = (Edge*)malloc(total_edges * sizeof(Edge));
    U32f edge_count = 0;

    // Fill edge array
    for (U32f i = 0; i < num_v; i++) {
        Node* node = graph->adjLists[i];
        while (node != nullptr) {
            U32f j = node->vertex;
            if (i < j) {
                edges[edge_count].u = i;
                edges[edge_count].v = j;
                edges[edge_count].weight = node->weight;
                edge_count++;
            }
            node = node->next;
        }
    }

    // Sort edges by weight
    qsort(edges, total_edges, sizeof(Edge), compare_edges);

    // Initialize Disjoint Set Union (Union-Find structure)
    DSU dsu(num_v);

    // Allocate memory for algorithm result
    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    cfg->res_kruskal->edges = (KruskalEdge*)malloc((num_v - 1) * sizeof(KruskalEdge));
    cfg->res_kruskal->num_edges = 0;

    // Start timing
    Timer timer;
    timer.start();

    // Main Kruskal loop
    for (U32f i = 0; i < total_edges; i++) {
        U32f u = edges[i].u;
        U32f v = edges[i].v;

        // If u and v are in different components, add the edge to MST
        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v);

            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].u = u;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].v = v;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].weight = edges[i].weight;
            cfg->res_kruskal->num_edges++;

            // Stop early if MST is complete
            if (cfg->res_kruskal->num_edges == num_v - 1) break;
        }
    }

    // Stop timing and store duration
    cfg->execution_time = timer.stop();

    // Free memory
    free(edges);
}

// Kruskal's algorithm using incidence matrix representation
void kruskal_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f density = cfg->density;
    U32f* inc_matrix = cfg->inc_matrix_undir;

    // Define edge structure (redeclared locally, identical to global struct)
    struct Edge {
        U32f u;
        U32f v;
        U32f weight;
    };

    // Allocate memory for edges
    Edge* edges = (Edge*)malloc(density * sizeof(Edge));
    U32f edge_count = 0;

    // Build edge list from incidence matrix (1 edge per column)
    for (U32f j = 0; j < density; j++) {
        U32f u = UINT32_MAX;
        U32f v = UINT32_MAX;
        U32f weight = 0;

        for (U32f i = 0; i < num_v; i++) {
            if (inc_matrix[i * density + j] != 0) {
                if (u == UINT32_MAX) {
                    u = i;
                    weight = inc_matrix[i * density + j];
                } else {
                    v = i;
                    break;
                }
            }
        }

        if (u != UINT32_MAX && v != UINT32_MAX) {
            edges[edge_count].u = u;
            edges[edge_count].v = v;
            edges[edge_count].weight = weight;
            edge_count++;
        }
    }

    // Sort edges by weight
    qsort(edges, edge_count, sizeof(Edge), compare_edges);

    // Initialize DSU
    DSU dsu(num_v);

    // Allocate memory for result
    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    cfg->res_kruskal->edges = (KruskalEdge*)malloc((num_v - 1) * sizeof(KruskalEdge));
    cfg->res_kruskal->num_edges = 0;

    // Start timing
    Timer timer;
    timer.start();

    // Main Kruskal loop
    for (U32f i = 0; i < edge_count; i++) {
        U32f u = edges[i].u;
        U32f v = edges[i].v;

        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v);

            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].u = u;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].v = v;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].weight = edges[i].weight;
            cfg->res_kruskal->num_edges++;

            if (cfg->res_kruskal->num_edges == num_v - 1) break;
        }
    }

    // Stop timer
    cfg->execution_time = timer.stop();

    // Free memory
    free(edges);
}
