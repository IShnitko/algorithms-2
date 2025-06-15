#include "dijkstra.h"

void dijkstra_list(Config* cfg) {
    printf("Dijkstra (list) algorithm running...\n");

    // Заглушка: создаем фиктивные результаты
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(cfg->num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(cfg->num_v * sizeof(U32f));

    for (U32f i = 0; i < cfg->num_v; i++) {
        cfg->res_sp->distances[i] = (i == cfg->start_vertex) ? 0 : i * 10;
        cfg->res_sp->parents[i] = (i == cfg->start_vertex) ? i : (i + 1) % cfg->num_v;
    }
}

void dijkstra_matrix(Config* cfg) {
    printf("Dijkstra (matrix) algorithm running...\n");
    dijkstra_list(cfg); // Для заглушки используем ту же реализацию
}