#include "prim.h"

void prim_list(Config* cfg) {
    printf("Prim (list) algorithm running...\n");

    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    cfg->res_prim->parent_weight = (PrimResult*)malloc(cfg->num_v * sizeof(PrimResult));

    for (U32f i = 0; i < cfg->num_v; i++) {
        if (i == cfg->start_vertex) {
            cfg->res_prim->parent_weight[i].parent = i;
            cfg->res_prim->parent_weight[i].weight = 0;
        } else {
            cfg->res_prim->parent_weight[i].parent = (i + 1) % cfg->num_v;
            cfg->res_prim->parent_weight[i].weight = i * 3;
        }
    }
}

void prim_matrix(Config* cfg) {
    printf("Prim (matrix) algorithm running...\n");
    prim_list(cfg);
}