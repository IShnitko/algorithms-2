#include "configuration.h"
#include "../graph/generators.h"
#include "../io/display.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <pstl/parallel_backend_utils.h>
#include "../utils/path_utils.h"
#include "../utils/random.h" // Добавляем для init_random

#define MAX_WEIGHT 4096

// Определение массива имен алгоритмов
const char* alg_names[ALG_TYPE_COUNT] = {
    "dijkstra_list",
    "dijkstra_matrix",
    "belman_ford_list",
    "belman_ford_matrix_edge_list",
    "belman_ford_matrix_no_edge_list",
    "prim_list",
    "prim_matrix",
    "kruskal_list",
    "kruskal_matrix"
};

// Упрощенная и улучшенная функция чтения конфигурации
void read_config_file(const char* file_name, File_config* cfg) {
    FILE* file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Cannot open config file: %s\n", file_name);
        return;
    }

    // Инициализация значений по умолчанию
    memset(cfg, 0, sizeof(File_config));
    // cfg->alg_type = UNKNOWN_ALG;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Пропускаем комментарии и пустые строки
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char key[128], value[128];
        if (sscanf(line, "%127s %127s", key, value) != 2) continue;

        // Удаляем возможные возвраты каретки
        char* cr = strchr(value, '\r');
        if (cr) *cr = '\0';
        cr = strchr(value, '\n');
        if (cr) *cr = '\0';

        if (strcmp(key, ".alg_type") == 0) {
            if (strcmp(value, "dijkstra_list") == 0) cfg->alg_type = DIJKSTRA_LIST;
            else if (strcmp(value, "dijkstra_matrix") == 0) cfg->alg_type = DIJKSTRA_MATRIX;
            else if (strcmp(value, "belman_ford_list") == 0) cfg->alg_type = BELMAN_FORD_LIST;
            else if (strcmp(value, "belman_ford_matrix_edge_list") == 0) cfg->alg_type = BELMAN_FORD_MATRIX_EDGE_LIST;
            else if (strcmp(value, "belman_ford_matrix_no_edge_list") == 0) cfg->alg_type = BELMAN_FORD_MATRIX_NO_EDGE_LIST;
            else if (strcmp(value, "prim_list") == 0) cfg->alg_type = PRIM_LIST;
            else if (strcmp(value, "prim_matrix") == 0) cfg->alg_type = PRIM_MATRIX;
            else if (strcmp(value, "kruskal_list") == 0) cfg->alg_type = KRUSKAL_LIST;
            else if (strcmp(value, "kruskal_matrix") == 0) cfg->alg_type = KRUSKAL_MATRIX;
            else fprintf(stderr, "Unknown algorithm type: %s\n", value);
        }
        else if (strcmp(key, ".file_name") == 0) {
            cfg->file_name = strdup(value);
        }
        else if (strcmp(key, ".start_vertex") == 0) {
            cfg->start_vertex = atoi(value);
        }
        else if (strcmp(key, ".out_list") == 0) {
            cfg->out_list = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, ".out_matrix") == 0) {
            cfg->out_matrix = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, ".num_v") == 0) {
            cfg->num_v = atoi(value);
        }
        else if (strcmp(key, ".density") == 0) {
            cfg->density = atof(value);
        }
    }
    fclose(file);
}

void print_config_file(File_config *cfg) {
    printf("==== CONFIGURATION ====\n");
    printf("Algorithm: %s\n", alg_names[cfg->alg_type]);
    printf("Vertices: %u\n", cfg->num_v);
    printf("Density: %.2f\n", cfg->density);
    printf("Start vertex: %u\n", cfg->start_vertex);
    printf("Output matrix: %s\n", cfg->out_matrix ? "true" : "false");
    printf("Output list: %s\n", cfg->out_list ? "true" : "false");
    printf("Input file: %s\n", cfg->file_name ? cfg->file_name : "none");
    printf("========================\n\n");
}

// Расчет плотности для ориентированного графа
static U32f density_dir(double density, U32f num_v) {
    return (U32f)(density * num_v * (num_v - 1));
}

// Расчет плотности для неориентированного графа
static U32f density_undir(double density, U32f num_v) {
    return (U32f)(density * num_v * (num_v - 1) / 2);
}

// Создание конфигурации со случайными весами
void create_config_random_weights(Config *cfg, U32f num_v, double density,
                                 enum Alg_type alg_type, U32f start_vertex) {
    // Проверки входных данных
    if (num_v == 0) {
        fprintf(stderr, "Vertex count must be greater than 0\n");
        exit(EXIT_FAILURE);
    }
    if (density < 0 || density > 1) {
        fprintf(stderr, "Density must be in [0, 1] range\n");
        exit(EXIT_FAILURE);
    }
    if (start_vertex >= num_v) {
        fprintf(stderr, "Start vertex must be less than vertex count\n");
        exit(EXIT_FAILURE);
    }

    // Инициализация конфигурации
    memset(cfg, 0, sizeof(Config));
    cfg->num_v = num_v;
    cfg->start_vertex = start_vertex;
    cfg->alg_type = alg_type;

    // Определяем тип графа
    bool directed = (
        alg_type == DIJKSTRA_LIST ||
        alg_type == DIJKSTRA_MATRIX ||
        alg_type == BELMAN_FORD_LIST ||
        alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST
    );

    // Рассчитываем количество ребер
    U32f target_edges = directed ? density_dir(density, num_v) : density_undir(density, num_v);
    cfg->density = target_edges;

    // Создаем граф
    cfg->graph = create_graph(num_v);
    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph structure\n");
        exit(EXIT_FAILURE);
    }

    // Генерируем граф
    if (directed) {
        create_rand_dir_graph(cfg->graph, target_edges, start_vertex);
        set_rand_weights_dir(cfg->graph, 1, MAX_WEIGHT);
    } else {
        create_rand_undir_graph(cfg->graph, target_edges);
        set_rand_weights_undir(cfg->graph, 1, MAX_WEIGHT);
    }

    // Создаем матричные представления при необходимости
    if (alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        cfg->inc_matrix_dir = create_inc_dir_matrix(cfg->graph, target_edges);
    }
    else if (alg_type == PRIM_MATRIX || alg_type == KRUSKAL_MATRIX) {
        cfg->inc_matrix_undir = create_inc_undir_matrix(cfg->graph, target_edges);
    }
}

// Создание конфигурации из существующего графа
void create_config_from_graph(Config *cfg, enum Alg_type alg_type,
                             U32f num_v, U32f density) {
    cfg->alg_type = alg_type;
    cfg->num_v = num_v;
    cfg->density = density;

    // Создаем матричные представления при необходимости
    if (alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        cfg->inc_matrix_dir = create_inc_dir_matrix(cfg->graph, density);
    }
    else if (alg_type == PRIM_MATRIX || alg_type == KRUSKAL_MATRIX) {
        cfg->inc_matrix_undir = create_inc_undir_matrix(cfg->graph, density);
    }
}

// Освобождение неиспользуемых ресурсов
void free_unused_config(Config *cfg, enum Alg_type alg_type) {
    // Для матричных алгоритмов освобождаем граф, если он не нужен
    bool is_matrix_alg = (
        alg_type == DIJKSTRA_MATRIX ||
        alg_type == BELMAN_FORD_MATRIX_EDGE_LIST ||
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST ||
        alg_type == PRIM_MATRIX ||
        alg_type == KRUSKAL_MATRIX
    );

    if (is_matrix_alg && cfg->graph) {
        free_graph(cfg->graph);
        cfg->graph = nullptr;
    }
}

// Освобождение конфигурации
void free_config(Config *cfg) {
    if (!cfg) return;

    if (cfg->graph) {
        free_graph(cfg->graph);
        cfg->graph = nullptr;
    }

    if (cfg->inc_matrix_dir) {
        free(cfg->inc_matrix_dir);
        cfg->inc_matrix_dir = nullptr;
    }

    if (cfg->inc_matrix_undir) {
        free(cfg->inc_matrix_undir);
        cfg->inc_matrix_undir = nullptr;
    }

    if (cfg->res_sp) {
        if (cfg->res_sp->distances) free(cfg->res_sp->distances);
        if (cfg->res_sp->parents) free(cfg->res_sp->parents);
        free(cfg->res_sp);
        cfg->res_sp = nullptr;
    }

    if (cfg->res_prim) {
        if (cfg->res_prim->parent_weight) free(cfg->res_prim->parent_weight);
        free(cfg->res_prim);
        cfg->res_prim = nullptr;
    }

    if (cfg->res_kruskal) {
        if (cfg->res_kruskal->edges) free(cfg->res_kruskal->edges);
        free(cfg->res_kruskal);
        cfg->res_kruskal = nullptr;
    }
}

// Освобождение конфигурации файла
void free_config_file(File_config* cfg) {
    if (!cfg) return;

    if (cfg->file_name) {
        free(cfg->file_name);
        cfg->file_name = nullptr;
    }
}
