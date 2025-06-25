#include "generators.h"
#include "graph.h"
#include "../utils/dsu.h"    // Disjoint Set Union for connected component checking.
#include "../utils/random.h" // For random number generation.
#include <vector>            // Standard C++ vector (might be used implicitly or by other headers).
#include <stdlib.h>          // For malloc, free, rand, srand.
#include <stdio.h>           // For printf, fprintf.

// Helper function to swap two U32f values.
static void swap(U32f *a, U32f *b) {
    U32f temp = *a;
    *a = *b;
    *b = temp;
}

// Shuffles two arrays (representing pairs of (u, v) vertices) simultaneously.
// This is used to randomize the order of potential edges.
// arr_u: Array of source vertices.
// arr_v: Array of destination vertices.
// n: Number of elements in the arrays.
static void shuffle(U32f *arr_u, U32f *arr_v, size_t n) {
    if (n <= 1) return; // No need to shuffle if 0 or 1 element.
    // Fisher-Yates shuffle algorithm.
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1); // Generate a random index from 0 to i.
        swap(&arr_u[i], &arr_u[j]);  // Swap arr_u[i] with arr_u[j].
        swap(&arr_v[i], &arr_v[j]);  // Swap arr_v[i] with arr_v[j] to keep pairs consistent.
    }
}

// Converts a Prüfer sequence into a list of edges representing a tree.
// seq: The Prüfer sequence.
// len_seq: The length of the Prüfer sequence (number of vertices - 2).
// Returns: An array of edges (u, v) pairs. The array size is 2 * (len_seq + 1)
//          because a tree with N vertices has N-1 edges, and len_seq = N-2.
//          So, edges = 2 * (N-2 + 1) = 2 * (N-1).
static U32f* prufer_to_tree(U32f *seq, U32f len_seq) {
    // A tree with N vertices has N-1 edges. If len_seq = N-2, then N-1 = len_seq + 1.
    // Each edge requires 2 U32f values, so 2 * (len_seq + 1) total U32f elements.
    U32f *edges = (U32f *)malloc(sizeof(U32f) * 2 * (len_seq + 1));
    if (edges == NULL) {
        fprintf(stderr, "Memory allocation failed for edges in prufer_to_tree.\n");
        exit(EXIT_FAILURE);
    }
    int i_edges = 0; // Index for current edge being added to 'edges' array.

    // 'length' represents the number of vertices (N).
    // If len_seq = N-2, then N = len_seq + 2.
    U32f length = len_seq + 2;
    U32f *degree = (U32f *)malloc(length * sizeof(U32f));
    if (degree == NULL) {
        fprintf(stderr, "Memory allocation failed for degree in prufer_to_tree.\n");
        free(edges);
        exit(EXIT_FAILURE);
    }

    // Initialize degrees: In a tree, every vertex initially has degree 1 (as if it's a leaf).
    for (U32f i = 0; i < length; i++) {
        degree[i] = 1;
    }

    // Update degrees based on the Prüfer sequence.
    // Each number in the sequence represents a non-leaf node, increasing its degree.
    for (U32f i = 0; i < len_seq; i++) {
        U32f node = seq[i];
        degree[node]++;
    }

    // Main loop to reconstruct the tree from the Prüfer sequence.
    // In each step, find the smallest degree-1 vertex (leaf) and connect it to
    // the current node in the sequence. Then decrement their degrees.
    for (U32f i = 0; i < len_seq; i++) {
        U32f node_seq = seq[i]; // Current node from the Prüfer sequence.
        for (U32f j = 0; j < length; j++) {
            if (degree[j] == 1) { // Found the smallest degree-1 vertex 'j'.
                edges[i_edges * 2] = node_seq;      // Add edge (node_seq, j).
                edges[i_edges * 2 + 1] = j;
                i_edges++;                          // Increment edge count.
                degree[j]--;                        // Decrement degree of 'j'.
                degree[node_seq]--;                 // Decrement degree of 'node_seq'.
                break; // Break inner loop to find next leaf.
            }
        }
    }

    // After the loop, two vertices will remain with degree 1. These form the last edge.
    U32f u = 0, v = 0;
    int first = 1; // Flag to find the first of the two remaining vertices.
    for (U32f i = 0; i < length; i++) {
        if (degree[i] == 1) {
            if (first) {
                u = i;
                first = 0;
            } else {
                v = i;
            }
        }
    }
    // Add the final edge.
    edges[i_edges * 2] = u;
    edges[i_edges * 2 + 1] = v;

    free(degree); // Free temporary degree array.
    return edges; // Return the array of tree edges.
}

// Generates a random Prüfer sequence for a tree with 'verticies' nodes.
// The length of the Prüfer sequence for N vertices is N-2.
// verticies: The total number of vertices in the tree.
// Returns: A dynamically allocated array containing the Prüfer sequence.
static U32f *prufer_rand_seq(U32f verticies) {
    // For N vertices, the Prüfer sequence has N-2 elements.
    // If verticies < 2, a tree cannot be formed, and (verticies - 2) would be problematic.
    // The prufer_to_tree expects len_seq = N-2, so for N=2, len_seq=0. For N=1, len_seq=-1 (problem).
    // The current usage for prufer_rand_seq is (verticies - 2) to match prufer_to_tree.
    // For graph generation, it expects verticies >= 2. If verticies = 2, length of seq is 0.
    // If verticies < 2, the behavior is undefined for prufer_to_tree(seq, verticies-2).
    // Assuming verticies >= 2.
    U32f seq_length = (verticies >= 2) ? (verticies - 2) : 0; // Ensure non-negative length.
    U32f* seq = (U32f *)malloc(sizeof(U32f) * seq_length);
    if (seq == NULL) {
        fprintf(stderr, "Memory allocation failed for seq in prufer_rand_seq.\n");
        exit(EXIT_FAILURE);
    }
    // Each element in the Prüfer sequence is a vertex index from 0 to 'verticies'-1.
    for (U32f i = 0; i < seq_length; i++) {
        seq[i] = rand() % verticies;
    }
    return seq;
}

// Creates a random connected undirected graph with a specified density.
// The graph is guaranteed to be connected.
// graph: Pointer to the Graph structure (adjacency list) to populate.
// density: The target number of edges (not a percentage here, but actual edge count).
void create_rand_undir_graph(Graph *graph, I32f density) {
    U32f verticies = graph->num_v; // Number of vertices in the graph.

    // Step 1: Generate a spanning tree using Prüfer sequences to ensure connectivity.
    // A tree with `verticies` nodes has `verticies - 1` edges.
    // The Prüfer sequence length is `verticies - 2`.
    U32f *seq = prufer_rand_seq(verticies);
    // If verticies < 2, prufer_rand_seq returns NULL or empty, handle carefully.
    // The condition (verticies - 2) as length for prufer_to_tree means it forms a tree with (verticies-2)+2 = verticies nodes.
    U32f *edges = prufer_to_tree(seq, verticies - 2); // Convert sequence to tree edges.
    free(seq); // Free the Prüfer sequence as it's no longer needed.

    // Add the edges of the generated tree to the graph.
    // Each edge is added twice for an undirected graph (u->v and v->u).
    for (U32f i = 0; i < verticies - 1; i++) {
        add_edge(graph, edges[i * 2], edges[i * 2 + 1], 0); // Add edge with dummy weight 0.
        add_edge(graph, edges[i * 2 + 1], edges[i * 2], 0); // Add reverse edge.
    }
    free(edges); // Free the temporary tree edges array.

    // Step 2: Add additional edges to reach the desired density.
    // Calculate the maximum possible edges for an undirected graph.
    U32f max_edges = verticies * (verticies - 1) / 2;
    // 'numb' is the count of potential edges that can be added without creating parallel edges
    // or self-loops, beyond the initial tree edges.
    U32f numb = max_edges - (verticies - 1); // Total non-tree edges available.

    // Allocate arrays to store candidate edges (edges not yet in the graph).
    U32f* candidates_u = (U32f *)malloc(sizeof(U32f) * numb);
    if (candidates_u == NULL) {
        fprintf(stderr, "Memory allocation failed for candidates_u in create_rand_undir_graph.\n");
        exit(EXIT_FAILURE);
    }
    U32f* candidates_v = (U32f *)malloc(sizeof(U32f) * numb);
    if (candidates_v == NULL) {
        fprintf(stderr, "Memory allocation failed for candidates_v in create_rand_undir_graph.\n");
        free(candidates_u);
        exit(EXIT_FAILURE);
    }
    U32f ind = 0; // Index for populating candidate arrays.

    // Populate candidate_u and candidate_v with all possible edges (i, j) where i < j
    // that are not already present in the graph.
    for (U32f i = 0; i < verticies; i++) {
        for (U32f j = i + 1; j < verticies; j++) { // Only consider i < j to avoid duplicates.
            if (check_edge(graph, i, j)) continue; // If edge (i, j) already exists, skip.
            candidates_u[ind] = i;
            candidates_v[ind] = j;
            ind++; // Increment count of candidate edges.
        }
    }

    // Shuffle the candidate edges to ensure random selection.
    shuffle(candidates_u, candidates_v, ind);

    // Adjust 'density' (which is target_edges from config) by subtracting the edges already added by the tree.
    density = density - (verticies - 1);

    // Ensure 'density' does not exceed the number of available candidate edges.
    if (density > ind) density = ind;
    // Ensure 'density' is not negative (if initial density was less than tree edges).
    else if (density < 0) density = 0;

    // Add the required number of additional edges from the shuffled candidates.
    for (U32f i = 0; i < density; i++) {
        add_edge(graph, candidates_u[i], candidates_v[i], 0); // Add edge with dummy weight 0.
        add_edge(graph, candidates_v[i], candidates_u[i], 0); // Add reverse edge.
    }

    // Free dynamically allocated candidate arrays.
    free(candidates_u);
    free(candidates_v);
}

// Recursively performs a Depth First Search (DFS) to build a directed spanning tree
// from a given start_vertex. This essentially finds parent pointers in the tree.
// graph: The graph being traversed (assumed to be a tree or connected graph).
// parent: Array to store the parent of each vertex in the DFS tree.
// start_vertex: The current vertex in the DFS.
// visited: Array to keep track of visited vertices during DFS.
static void get_dir_tree(Graph *graph, U32f *parent, U32f start_vertex, U32f *visited) {
    visited[start_vertex] = 1; // Mark the current vertex as visited.
    // Iterate through all neighbors of the current vertex.
    for (Node *temp = graph->adjLists[start_vertex]; temp != NULL; temp = temp->next) {
        // If a neighbor has not been visited, recursively call DFS on it
        // and set the current vertex as its parent.
        if (!visited[temp->vertex]) {
            parent[temp->vertex] = start_vertex; // Set parent.
            get_dir_tree(graph, parent, temp->vertex, visited); // Recursive call.
        }
    }
}

// Creates a random connected directed graph with a specified density,
// ensuring reachability from a given start_vertex.
// graph: Pointer to the Graph structure (adjacency list) to populate.
// density: The target number of edges.
// start_vertex: The starting vertex from which all other vertices should be reachable.
void create_rand_dir_graph(Graph *graph, I32f density, U32f start_vertex) {
    U32f verticies = graph->num_v; // Number of vertices.

    // Step 1: Generate an undirected spanning tree to ensure connectivity.
    // This tree will then be oriented to form a directed tree from start_vertex.
    U32f *seq = prufer_rand_seq(verticies);
    U32f *edges = prufer_to_tree(seq, verticies - 2);
    free(seq); // Free Prüfer sequence.

    // Create a temporary graph to build the undirected tree.
    Graph *t_graph = create_graph(verticies);
    if (!t_graph) {
        fprintf(stderr, "Failed to create temporary graph structure.\n");
        free(edges);
        exit(EXIT_FAILURE);
    }
    // Add edges of the generated tree to the temporary graph (undirected).
    for (U32f i = 0; i < verticies - 1; i++) {
        add_edge(t_graph, edges[i * 2], edges[i * 2 + 1], 0);
        add_edge(t_graph, edges[i * 2 + 1], edges[i * 2], 0);
    }
    free(edges); // Free temporary tree edges.

    // Step 2: Orient the tree edges to form a directed tree rooted at start_vertex.
    U32f *parent = (U32f *)malloc(verticies * sizeof(U32f));
    if (parent == NULL) {
        fprintf(stderr, "Memory allocation failed for parent in create_rand_dir_graph.\n");
        free_graph(t_graph);
        exit(EXIT_FAILURE);
    }
    U32f *visited = (U32f *)calloc(verticies, sizeof(U32f)); // Initialize to 0 (unvisited).
    if (visited == NULL) {
        fprintf(stderr, "Memory allocation failed for visited in create_rand_dir_graph.\n");
        free(parent);
        free_graph(t_graph);
        exit(EXIT_FAILURE);
    }
    // Perform DFS from start_vertex to populate the 'parent' array, forming a directed tree.
    get_dir_tree(t_graph, parent, start_vertex, visited);

    free(visited);     // Free temporary visited array.
    free_graph(t_graph); // Free the temporary undirected graph.

    // Add the directed tree edges to the main 'graph'.
    // For each vertex 'i' (except the start_vertex), add a directed edge from its parent to 'i'.
    for (U32f i = 0; i < verticies; i++) {
        if (i != start_vertex) { // Start vertex has no parent in the tree.
            add_edge(graph, parent[i], i, 0); // Add edge (parent[i] -> i) with dummy weight 0.
        }
    }
    free(parent); // Free the parent array.

    // Step 3: Add additional random directed edges to reach the desired density.
    // Maximum possible directed edges (excluding self-loops) = V * (V - 1).
    U32f max_edges = verticies * (verticies - 1);
    // 'numb' is the count of possible additional edges beyond the tree.
    U32f numb = max_edges - (verticies - 1); // Total non-tree edges available.

    U32f *candidates_u = (U32f *)malloc(sizeof(U32f) * numb);
    if (candidates_u == NULL) {
        fprintf(stderr, "Memory allocation failed for candidates_u in create_rand_dir_graph (final).\n");
        exit(EXIT_FAILURE);
    }
    U32f *candidates_v = (U32f *)malloc(sizeof(U32f) * numb);
    if (candidates_v == NULL) {
        fprintf(stderr, "Memory allocation failed for candidates_v in create_rand_dir_graph (final).\n");
        free(candidates_u);
        exit(EXIT_FAILURE);
    }
    I32f ind = 0; // Index for populating candidate arrays.

    // Populate candidate_u and candidate_v with all possible directed edges (i -> j)
    // that are not self-loops and not already present in the graph.
    for (U32f i = 0; i < verticies; i++) {
        for (U32f j = 0; j < verticies; j++) {
            if (i == j || check_edge(graph, i, j)) continue; // Skip self-loops and existing edges.
            candidates_u[ind] = i;
            candidates_v[ind] = j;
            ind++;
        }
    }

    // Shuffle the candidate edges to ensure random selection.
    shuffle(candidates_u, candidates_v, ind);

    // Adjust 'density' by subtracting the edges already added by the tree.
    density = density - (verticies - 1);

    // Ensure 'density' does not exceed the number of available candidate edges.
    if (ind > 0 && density > ind) density = ind;
    // Ensure 'density' is not negative.
    else if (density < 0) density = 0;

    // Add the required number of additional directed edges from the shuffled candidates.
    for (U32f i = 0; i < density; i++) {
        add_edge(graph, candidates_u[i], candidates_v[i], 0); // Add edge with dummy weight 0.
    }

    // Free dynamically allocated candidate arrays.
    free(candidates_u);
    free(candidates_v);
}

// Creates an undirected incidence matrix from an adjacency list graph.
// graph: Pointer to the Graph structure (adjacency list).
// density: The number of edges in the graph (which will be the number of columns in the matrix).
// Returns: A dynamically allocated U32f array representing the undirected incidence matrix.
//          Returns NULL if memory allocation fails.
U32f *create_inc_undir_matrix(Graph* graph, U32f density) {
    printf("Creating undirected incidence matrix: vertices=%lu, density=%lu\n", graph->num_v, density);

    // Allocate memory for the incidence matrix. Size: num_v rows * density columns.
    U32f *inc_matrix = (U32f *)calloc(graph->num_v * density, sizeof(U32f)); // calloc initializes to 0.
    if (!inc_matrix) {
        fprintf(stderr, "Memory allocation failed for incidence matrix in create_inc_undir_matrix.\n");
        return NULL;
    }

    U32f ind = 0; // Current column index for the incidence matrix (represents an edge).
    // Iterate through all vertices 'i'.
    for (U32f i = 0; i < graph->num_v; i++) {
        // Iterate through all neighbors of 'i'.
        Node* temp = graph->adjLists[i];
        while (temp) {
            U32f j = temp->vertex; // Neighbor vertex.
            // For undirected graphs, each edge (i,j) is stored twice in adjLists (i->j and j->i).
            // To avoid adding the same edge twice to the matrix, only consider (i,j) where i < j.
            if (i < j) {
                inc_matrix[i * density + ind] = temp->weight; // Set entry for vertex 'i' in edge 'ind'.
                inc_matrix[j * density + ind] = temp->weight; // Set entry for vertex 'j' in edge 'ind'.
                ind++; // Move to the next edge column.
            }
            temp = temp->next;
        }
    }
    return inc_matrix;
}

// Creates a directed incidence matrix from an adjacency list graph.
// graph: Pointer to the Graph structure (adjacency list).
// density: The number of edges in the graph (number of columns in the matrix).
// Returns: A dynamically allocated I32f array representing the directed incidence matrix.
//          Returns NULL if memory allocation fails.
I32f *create_inc_dir_matrix(Graph* graph, U32f density) {
    printf("Creating directed incidence matrix: vertices=%lu, density=%lu\n", graph->num_v, density);

    // Allocate memory for the incidence matrix. Size: num_v rows * density columns.
    I32f *inc_matrix = (I32f *)calloc(graph->num_v * density, sizeof(I32f)); // calloc initializes to 0.
    if (!inc_matrix) {
        fprintf(stderr, "Memory allocation failed for incidence matrix in create_inc_dir_matrix.\n");
        return NULL;
    }

    U32f ind = 0; // Current column index for the incidence matrix (represents an edge).
    // Iterate through all vertices 'i'.
    for (U32f i = 0; i < graph->num_v; i++) {
        // Iterate through all neighbors of 'i'.
        Node* temp = graph->adjLists[i];
        while (temp) {
            U32f j = temp->vertex; // Neighbor vertex (destination).
            // Ensure no self-loops are considered (though graph generation should prevent this).
            if (i != j) {
                inc_matrix[i * density + ind] = -(I32f)temp->weight; // Source vertex 'i' gets -weight.
                inc_matrix[j * density + ind] = (I32f)temp->weight;  // Destination vertex 'j' gets +weight.
                ind++; // Move to the next edge column.
            }
            temp = temp->next;
        }
    }
    return inc_matrix;
}

// Generates a random connected graph (either directed or undirected) with a specified density.
// This function aims for strict connectivity, first building a spanning tree, then adding more edges.
// vertices: The number of vertices in the graph.
// density: The desired density (as a fraction, 0.0 to 1.0) of additional edges beyond the spanning tree.
//          Note: The density parameter here is used differently from `create_rand_undir_graph` / `_dir_graph`.
//                It's a fraction of *remaining* possible edges after a tree is formed.
// directed: Boolean flag; true for a directed graph, false for undirected.
// Returns: A pointer to the newly created Graph structure (adjacency list), or nullptr on failure.
Graph* generate_connected_graph(U32f vertices, double density, bool directed) {
    if (vertices == 0) return nullptr;

    // Calculate maximum possible edges and minimum edges for connectivity (V-1 for a tree).
    U32f max_edges = directed ? vertices * (vertices - 1) : vertices * (vertices - 1) / 2;
    U32f min_edges = vertices - 1; // Minimum edges for a connected graph (a spanning tree).

    // Calculate the target total number of edges.
    // It's `min_edges` plus a percentage (`density`) of the *remaining* possible edges.
    U32f target_edges = min_edges + (U32f)((max_edges - min_edges) * density);

    // Create the graph structure.
    Graph* graph = create_graph(vertices);
    if (!graph) {
        fprintf(stderr, "Failed to create graph in generate_connected_graph.\n");
        return nullptr;
    }

    // Step 1: Create a spanning tree to guarantee connectivity.
    // Use Disjoint Set Union (DSU) to efficiently check and maintain connectivity.
    DSU dsu(vertices);
    U32f edges_added = 0; // Counter for edges added so far.

    // Keep adding edges until a spanning tree (min_edges) is formed.
    while (edges_added < min_edges) {
        U32f u = rand_range(0, vertices - 1); // Random source vertex.
        U32f v = rand_range(0, vertices - 1); // Random destination vertex.

        // Ensure u and v are distinct and not already in the same connected component.
        if (u != v && dsu.find(u) != dsu.find(v)) {
            U32f weight = rand_range(1, 100); // Random weight for the edge.
            add_edge(graph, u, v, weight);
            if (!directed) {
                add_edge(graph, v, u, weight); // Add reverse edge for undirected.
            }
            dsu.unite(u, v); // Union the sets of u and v.
            edges_added++;   // Increment edge count.
        }
    }

    // Step 2: Add remaining edges until the target density is reached.
    // Add edges randomly as long as they don't already exist and total edges < target_edges.
    while (edges_added < target_edges) {
        U32f u = rand_range(0, vertices - 1);
        U32f v = rand_range(0, vertices - 1);

        // Ensure u and v are distinct and the edge (u,v) does not already exist.
        if (u != v && !check_edge(graph, u, v)) {
            U32f weight = rand_range(1, 100);
            add_edge(graph, u, v, weight);
            if (!directed) {
                add_edge(graph, v, u, weight);
            }
            edges_added++;
        }
    }

    return graph; // Return the generated graph.
}
