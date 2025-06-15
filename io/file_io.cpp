#include "file_io.h"
#include "../config/configuration.h"
#include "../graph/graph.h"
#include "../io/display.h"
#include "../algorithms/sp/dijkstra.h"
#include "../algorithms/sp/bellman_ford.h"
#include "../algorithms/mst/prim.h"
#include "../algorithms/mst/kruskal.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define BREAK_LINE "--------------------------------\n"
#define SECTION_LINE "================================\n"

// Загрузка ориентированного графа
static void load_dir_graph(const char *file_name, Config *cfg) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        exit(1);
    }

    U32f num_v, num_e;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fclose(file);
        fprintf(stderr, "Invalid file format\n");
        exit(1);
    }

    cfg->num_v = num_v;
    cfg->density = num_e;
    cfg->graph = create_graph(num_v);

    if (!cfg->graph) {
        fclose(file);
        fprintf(stderr, "Failed to create graph\n");
        exit(1);
    }

    U32f src, dst, weight;
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Invalid vertex index: %u->%u\n", src, dst);
            continue;
        }
        add_edge(cfg->graph, src, dst, weight);
    }

    fclose(file);
}

// Загрузка неориентированного графа
static void load_undir_graph(const char *file_name, Config* cfg) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        exit(1);
    }

    U32f num_v, num_e;
    if (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32, &num_v, &num_e) != 2) {
        fclose(file);
        fprintf(stderr, "Invalid file format\n");
        exit(1);
    }

    cfg->num_v = num_v;
    cfg->density = num_e;
    cfg->graph = create_graph(num_v);

    if (!cfg->graph) {
        fclose(file);
        fprintf(stderr, "Failed to create graph\n");
        exit(1);
    }

    U32f src, dst, weight;
    while (fscanf(file, "%" SCNuFAST32 " %" SCNuFAST32 " %" SCNuFAST32, &src, &dst, &weight) == 3) {
        if (src >= num_v || dst >= num_v) {
            fprintf(stderr, "Invalid vertex index: %u-%u\n", src, dst);
            continue;
        }
        add_edge(cfg->graph, src, dst, weight);
        add_edge(cfg->graph, dst, src, weight);
    }

    fclose(file);
}

// Загрузка графа
void load_graph_from_file(const char *file_name, File_config *cfg_file, Config* cfg) {
    if (cfg_file->alg_type == DIJKSTRA_LIST ||
        cfg_file->alg_type == DIJKSTRA_MATRIX ||
        cfg_file->alg_type == BELMAN_FORD_LIST ||
        cfg_file->alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        cfg_file->alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        load_dir_graph(file_name, cfg);
    }
    else {
        load_undir_graph(file_name, cfg);
    }
}

// Вывод представлений графа
void print_graph_representation(const Config *cfg, const File_config *cfg_file) {
    if (cfg_file->out_matrix) {
        if (cfg->inc_matrix_dir) {
            printf("Directed Incidence Matrix:\n");
            print_inc_dir_matrix(cfg->inc_matrix_dir, cfg->num_v, cfg->density);
            printf(BREAK_LINE);
        }
        else if (cfg->inc_matrix_undir) {
            printf("Undirected Incidence Matrix:\n");
            print_inc_undir_matrix(cfg->inc_matrix_undir, cfg->num_v, cfg->density);
            printf(BREAK_LINE);
        }
    }

    if (cfg_file->out_list && cfg->graph) {
        printf("Adjacency List:\n");
        print_graph(cfg->graph);
        printf(BREAK_LINE);
    }
}

// Запуск с случайным графом
void run_config_file_var(File_config *cfg_file, Config *cfg) {
    printf("1. Generate random graph and run algorithm\n");
    printf(BREAK_LINE);

    // Создаем конфигурацию со случайным графом
    create_config_random_weights(cfg, cfg_file->num_v, cfg_file->density,
                                cfg_file->alg_type, cfg_file->start_vertex);

    // Выводим представления графа
    print_graph_representation(cfg, cfg_file);

    // Освобождаем неиспользуемые ресурсы
    free_unused_config(cfg, cfg->alg_type);

    // Выбираем и выполняем алгоритм
    switch (cfg->alg_type) {
        case DIJKSTRA_LIST:
            dijkstra_list(cfg);
            break;
        case DIJKSTRA_MATRIX:
            dijkstra_matrix(cfg);
            break;
        case BELMAN_FORD_LIST:
            bellman_ford_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_EDGE_LIST:
            bellman_ford_matrix_edge_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_NO_EDGE_LIST:
            bellman_ford_matrix_no_edge_list(cfg);
            break;
        case PRIM_LIST:
            prim_list(cfg);
            break;
        case PRIM_MATRIX:
            prim_matrix(cfg);
            break;
        case KRUSKAL_LIST:
            kruskal_list(cfg);
            break;
        case KRUSKAL_MATRIX:
            kruskal_matrix(cfg);
            break;
        default:
            fprintf(stderr, "Unknown algorithm type\n");
            break;
    }

    // Выводим результаты
    printf("Results for %s:\n", alg_names[cfg_file->alg_type]);

    // Вывод результатов в зависимости от типа алгоритма
    if (cfg->res_sp) {
        printf("Distances:\n");
        for (U32f i = 0; i < cfg->num_v; i++) {
            printf("  to %u: %u (parent: %u)\n", i, cfg->res_sp->distances[i], cfg->res_sp->parents[i]);
        }
    }

    if (cfg->res_prim) {
        printf("MST (Prim):\n");
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (i == cfg->start_vertex) {
                printf("  [root] %u\n", i);
                continue;
            }
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_prim->parent_weight[i].parent,
                   i,
                   cfg->res_prim->parent_weight[i].weight);
        }
    }

    if (cfg->res_kruskal) {
        printf("MST (Kruskal):\n");
        for (U32f i = 0; i < cfg->res_kruskal->num_edges; i++) {
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_kruskal->edges[i].u,
                   cfg->res_kruskal->edges[i].v,
                   cfg->res_kruskal->edges[i].weight);
        }
    }

    printf(SECTION_LINE);
}

// Запуск с графом из файла
void run_config_file_load(File_config *cfg_file, Config *cfg) {
    printf("2. Load graph from file and run algorithm\n");
    printf(BREAK_LINE);

    if (!cfg_file->file_name || strlen(cfg_file->file_name) == 0) {
        fprintf(stderr, "No input file specified\n");
        return;
    }

    load_graph_from_file(cfg_file->file_name, cfg_file, cfg);

    if (!cfg->graph) {
        printf("Error loading graph from %s\n", cfg_file->file_name);
        return;
    }

    create_config_from_graph(cfg, cfg_file->alg_type, cfg_file->num_v, cfg->density);
    print_graph_representation(cfg, cfg_file);
    free_unused_config(cfg, cfg->alg_type);

    // Тот же switch-case для вызова алгоритма
    switch (cfg->alg_type) {
        case DIJKSTRA_LIST:
            dijkstra_list(cfg);
            break;
        case DIJKSTRA_MATRIX:
            dijkstra_matrix(cfg);
            break;
        case BELMAN_FORD_LIST:
            bellman_ford_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_EDGE_LIST:
            bellman_ford_matrix_edge_list(cfg);
            break;
        case BELMAN_FORD_MATRIX_NO_EDGE_LIST:
            bellman_ford_matrix_no_edge_list(cfg);
            break;
        case PRIM_LIST:
            prim_list(cfg);
            break;
        case PRIM_MATRIX:
            prim_matrix(cfg);
            break;
        case KRUSKAL_LIST:
            kruskal_list(cfg);
            break;
        case KRUSKAL_MATRIX:
            kruskal_matrix(cfg);
            break;
        default:
            fprintf(stderr, "Unknown algorithm type\n");
            break;
    }

    // Выводим результаты
    printf("Results for %s:\n", alg_names[cfg_file->alg_type]);

    if (cfg->res_sp) {
        printf("Distances:\n");
        for (U32f i = 0; i < cfg->num_v; i++) {
            printf("  to %u: %u (parent: %u)\n", i, cfg->res_sp->distances[i], cfg->res_sp->parents[i]);
        }
    }

    if (cfg->res_prim) {
        printf("MST (Prim):\n");
        for (U32f i = 0; i < cfg->num_v; i++) {
            if (i == cfg->start_vertex) {
                printf("  [root] %u\n", i);
                continue;
            }
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_prim->parent_weight[i].parent,
                   i,
                   cfg->res_prim->parent_weight[i].weight);
        }
    }

    if (cfg->res_kruskal) {
        printf("MST (Kruskal):\n");
        for (U32f i = 0; i < cfg->res_kruskal->num_edges; i++) {
            printf("  %u - %u (weight: %u)\n",
                   cfg->res_kruskal->edges[i].u,
                   cfg->res_kruskal->edges[i].v,
                   cfg->res_kruskal->edges[i].weight);
        }
    }

    printf(SECTION_LINE);
}