#include "dsu.h"

// Constructor: initialize DSU for n elements
DSU::DSU(U32f n) {
    size = n;
    parent = (U32f*)malloc(n * sizeof(U32f));  // Allocate memory for parent array
    rank = (U32f*)calloc(n, sizeof(U32f));     // Allocate and zero-initialize rank array

    // Initially, each element is its own parent (self root)
    for (U32f i = 0; i < n; i++) {
        parent[i] = i;
    }
}

// Destructor: free allocated memory
DSU::~DSU() {
    free(parent);
    free(rank);
}

// Find with path compression: returns the root of element u
U32f DSU::find(U32f u) {
    if (parent[u] != u) {
        parent[u] = find(parent[u]); // Path compression optimization
    }
    return parent[u];
}

// Union by rank: merges sets containing u and v
void DSU::unite(U32f u, U32f v) {
    u = find(u);
    v = find(v);

    if (u == v) return; // Already in the same set

    // Attach smaller rank tree under root of higher rank tree
    if (rank[u] < rank[v]) {
        parent[u] = v;
    } else if (rank[u] > rank[v]) {
        parent[v] = u;
    } else {
        parent[v] = u;
        rank[u]++; // Increase rank if both have same rank
    }
}
