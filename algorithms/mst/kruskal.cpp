#include "kruskal.h"

void kruskal_list(Config* cfg) {
    printf("Kruskal (list) algorithm running...\n");

    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    cfg->res_kruskal->num_edges = cfg->num_v - 1;
    cfg->res_kruskal->edges = (KruskalEdge*)malloc(cfg->res_kruskal->num_edges * sizeof(KruskalEdge));
    
    for (U32f i = 0; i < cfg->res_kruskal->num_edges; i++) {
        cfg->res_kruskal->edges[i].u = i;
        cfg->res_kruskal->edges[i].v = i + 1;
        cfg->res_kruskal->edges[i].weight = i * 4;
    }
}

void kruskal_matrix(Config* cfg) {
    printf("Kruskal (matrix) algorithm running...\n");
    kruskal_list(cfg);
}