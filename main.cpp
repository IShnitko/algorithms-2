#include "graph/graph.h"
#include "graph/generators.h"
#include "io/display.h"
#include "utils/timer.h"
#include "utils/random.h"
#include <cstdio>
#include <cstdlib>

int main() {
    init_random();

    // Parameters
    const U32f vertices = 5;
    const double density_percent = 50.0; // 50% density
    const U32f min_weight = 1;
    const U32f max_weight = 100;

    // Create graph
    Graph* graph = create_graph(vertices);
    if (!graph) {
        fprintf(stderr, "Failed to create graph\n");
        return 1;
    }

    // Generate undirected graph
    Timer timer;
    timer.start();
    create_rand_undir_graph(graph, static_cast<I32f>((density_percent / 100) * (vertices * (vertices - 1) / 2)));
    double gen_time = timer.stop();

    // Set random weights
    set_rand_weights_undir(graph, min_weight, max_weight);

    printf("=== Generated undirected graph (%u vertices, %.1f%%) ===\n",
           vertices, density_percent);
    printf("Generation time: %.3f ms\n\n", gen_time);

    // Print representations
    printf("--- Adjacency list ---\n");
    print_graph(graph);

    const U32f edges_count = static_cast<U32f>((density_percent / 100) * (vertices * (vertices - 1) / 2));
    printf("\n--- Incidence matrix (%u edges) ---\n", edges_count);
    U32f* inc_matrix = create_inc_undir_matrix(graph, edges_count);
    if (inc_matrix) {
        print_inc_undir_matrix(inc_matrix, vertices, edges_count);
        free(inc_matrix);
    }

    // Cleanup
    free_graph(graph);

    return 0;
}