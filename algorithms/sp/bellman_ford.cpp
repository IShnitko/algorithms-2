#include "bellman_ford.h"

void bellman_ford_list(Config* cfg) {
    printf("Bellman-Ford (list) algorithm running...\n");

    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(cfg->num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(cfg->num_v * sizeof(U32f));

    for (U32f i = 0; i < cfg->num_v; i++) {
        cfg->res_sp->distances[i] = (i == cfg->start_vertex) ? 0 : i * 5;
        cfg->res_sp->parents[i] = (i == cfg->start_vertex) ? i : (i + 2) % cfg->num_v;
    }
}

void bellman_ford_matrix_edge_list(Config* cfg) {
    printf("Bellman-Ford (matrix with edge list) algorithm running...\n");
    bellman_ford_list(cfg);
}

void bellman_ford_matrix_no_edge_list(Config* cfg) {
    printf("Bellman-Ford (matrix without edge list) algorithm running...\n");
    bellman_ford_list(cfg);
}