#include "dsu.h"

DSU::DSU(U32f n) {
    size = n;
    parent = (U32f*)malloc(n * sizeof(U32f));
    rank = (U32f*)calloc(n, sizeof(U32f));
    for (U32f i = 0; i < n; i++) {
        parent[i] = i;
    }
}

DSU::~DSU() {
    free(parent);
    free(rank);
}

U32f DSU::find(U32f u) {
    if (parent[u] != u) {
        parent[u] = find(parent[u]); // Path compression
    }
    return parent[u];
}

void DSU::unite(U32f u, U32f v) {
    u = find(u);
    v = find(v);
    
    if (u == v) return;
    
    // Union by rank
    if (rank[u] < rank[v]) {
        parent[u] = v;
    } else if (rank[u] > rank[v]) {
        parent[v] = u;
    } else {
        parent[v] = u;
        rank[u]++;
    }
}