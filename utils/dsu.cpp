#include "dsu.h" // Include the header file for the DSU class.
#include <stdlib.h> // Required for malloc and free.
#include <stdio.h>  // Required for fprintf (for error messages, though not explicitly used here).

// Constructor for the Disjoint Set Union (DSU) data structure.
// Initializes 'n' sets, where each element is initially in its own set.
// n: The number of elements (vertices) to manage.
DSU::DSU(U32f n) {
    size = n; // Store the number of elements.
    // Allocate memory for the 'parent' array.
    parent = (U32f*)malloc(n * sizeof(U32f));
    if (parent == NULL) {
        // Handle memory allocation failure.
        fprintf(stderr, "Error: Memory allocation failed for DSU parent array.\n");
        // In a real application, you might throw an exception or handle this more gracefully.
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the 'rank' array and initialize it to zeros using calloc.
    rank = (U32f*)calloc(n, sizeof(U32f));
    if (rank == NULL) {
        // Handle memory allocation failure for rank array.
        fprintf(stderr, "Error: Memory allocation failed for DSU rank array.\n");
        free(parent); // Free parent array if rank allocation fails.
        exit(EXIT_FAILURE);
    }

    // Initialize each element to be its own parent, effectively creating 'n' individual sets.
    for (U32f i = 0; i < n; i++) {
        parent[i] = i;
    }
}

// Destructor for the DSU class.
// Frees the dynamically allocated memory for 'parent' and 'rank' arrays.
DSU::~DSU() {
    free(parent); // Free parent array.
    free(rank);   // Free rank array.
    parent = NULL; // Set pointers to NULL to prevent dangling pointers.
    rank = NULL;
}

// Finds the representative (root) of the set containing element 'u'.
// Implements path compression for optimization.
// u: The element whose set representative is to be found.
// Returns: The representative (root) of the set containing 'u'.
U32f DSU::find(U32f u) {
    // If 'u' is not its own parent, it's not the root of its set.
    if (parent[u] != u) {
        // Recursively find the root and perform path compression:
        // Make 'u' directly point to the root of its set.
        parent[u] = find(parent[u]);
    }
    return parent[u]; // Return the representative.
}

// Unites the sets containing elements 'u' and 'v'.
// Implements union by rank for optimization.
// u: An element in the first set.
// v: An element in the second set.
void DSU::unite(U32f u, U32f v) {
    // Find the representatives of the sets containing 'u' and 'v'.
    u = find(u);
    v = find(v);

    // If 'u' and 'v' are already in the same set, do nothing.
    if (u == v) return;

    // Perform union by rank: attach the smaller rank tree under the root of the larger rank tree.
    // This helps keep the trees flatter.
    if (rank[u] < rank[v]) {
        parent[u] = v; // 'u's set becomes a child of 'v's set.
    } else if (rank[u] > rank[v]) {
        parent[v] = u; // 'v's set becomes a child of 'u's set.
    } else {
        // If ranks are equal, attach one to the other and increment the rank of the new root.
        parent[v] = u; // 'v's set becomes a child of 'u's set.
        rank[u]++;     // Increment rank of 'u' because its height effectively increased.
    }
}
