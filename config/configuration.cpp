#include "configuration.h"
#include "../graph/generators.h"
#include "../io/display.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <pstl/parallel_backend_utils.h>

#include "../utils/path_utils.h"
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

// Парсинг типа алгоритма
static int parse_alg_type(const char* line, File_config* cfg) {
    for (int i = 0; i < ALG_TYPE_COUNT; i++) {
        if (strcmp(line, alg_names[i]) == 0) {
            cfg->alg_type = (enum Alg_type)i;
            return 1;
        }
    }
    fprintf(stderr, "Unknown algorithm type: %s\n", line);
    return 0;
}

// Парсинг количества вершин
static int parse_num_v(const char* line, File_config* cfg) {
    // Если уже указано имя файла, игнорируем num_v
    if (cfg->file_name) return 1;

    int num_v = atoi(line);
    if (num_v <= 0) {
        fprintf(stderr, "Invalid vertex count: %s\n", line);
        return 0;
    }
    cfg->num_v = (U32f)num_v;
    return 1;
}

// Парсинг плотности
static int parse_density(const char* line, File_config* cfg) {
    // Если уже указано имя файла, игнорируем density
    if (cfg->file_name) return 1;

    double density = atof(line);
    if (density <= 0.0 || density > 1.0) {
        fprintf(stderr, "Invalid density: %s\n", line);
        return 0;
    }
    cfg->density = density;
    return 1;
}

// Парсинг стартовой вершины
static int parse_start_vertex(const char* line, File_config* cfg) {
    int start_vertex = atoi(line);
    if (start_vertex < 0) {
        fprintf(stderr, "Invalid start vertex: %s\n", line);
        return 0;
    }
    cfg->start_vertex = (U32f)start_vertex;
    return 1;
}

// Парсинг флагов вывода
static int parse_out_flag(const char* line, int* flag) {
    if (strcmp(line, "true") == 0) {
        *flag = 1;
        return 1;
    } else if (strcmp(line, "false") == 0) {
        *flag = 0;
        return 1;
    }
    fprintf(stderr, "Invalid flag value: %s\n", line);
    return 0;
}

// Парсинг имени файла
static int parse_file_name(const char* line, File_config* cfg) {
    if (line == NULL || strlen(line) == 0) {
        fprintf(stderr, "Invalid file name\n");
        return 0;
    }
    cfg->file_name = strdup(line);
    return 1;
}

// Чтение конфигурационного файла

void read_config_file(const char* file_name, File_config* cfg) {
    FILE* file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Cannot open config file: %s\n", file_name);
        return;
    }

    // Инициализация значений по умолчанию
    // cfg->alg_type = UNKNOWN_ALG;
    cfg->file_name = nullptr;
    cfg->start_vertex = 0;
    cfg->out_list = false;
    cfg->out_matrix = false;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Пропускаем пустые строки и комментарии
        if (line[0] == '\n' || line[0] == '#') continue;

        char key[128], value[128];
        if (sscanf(line, "%127s %127s", key, value) != 2) continue;

        if (strcmp(key, ".alg_type") == 0) {
            if (strcmp(value, "dijkstra_list") == 0) cfg->alg_type = DIJKSTRA_LIST;
            // Добавьте другие алгоритмы по необходимости
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
    }

    fclose(file);
}

void print_config_file(const File_config* cfg) {
    printf("==== CONFIGURATION ====\n");
    printf("Algorithm: %s\n", alg_names[cfg->alg_type]);
    printf("Start vertex: %u\n", cfg->start_vertex);
    printf("Output list: %s\n", cfg->out_list ? "true" : "false");
    printf("Output matrix: %s\n", cfg->out_matrix ? "true" : "false");
    printf("Input file: %s\n", cfg->file_name ? cfg->file_name : "none");
    printf("========================\n\n");
}

// Расчет плотности для ориентированного графа
static U32f density_dir(double density, U32f num_v) {
    return (U32f)(num_v * (num_v - 1) * density);
}

// Расчет плотности для неориентированного графа
static U32f density_undir(double density, U32f num_v) {
    return (U32f)(num_v * (num_v - 1) / 2 * density);
}

// Создание конфигурации со случайными весами
void create_config_random_weights(Config *cfg, U32f num_v, double density, 
                                 enum Alg_type alg_type, U32f start_vertex) {
    printf("Creating config: num_v=%u, density=%f\n", num_v, density);
    // Проверки входных данных
    if (start_vertex >= num_v) {
        fprintf(stderr, "Start vertex must be less than vertex count\n");
        exit(EXIT_FAILURE);
    }
    if (density < 0 || density > 1) {
        fprintf(stderr, "Density must be in [0, 1] range\n");
        exit(EXIT_FAILURE);
    }
    if (alg_type < 0 || alg_type >= ALG_TYPE_COUNT) {
        fprintf(stderr, "Unknown algorithm type\n");
        exit(EXIT_FAILURE);
    }
    
    // Инициализация конфигурации
    memset(cfg, 0, sizeof(Config));
    cfg->num_v = num_v;
    cfg->start_vertex = start_vertex;
    cfg->alg_type = alg_type;
    cfg->graph = create_graph(num_v);
    
    if (!cfg->graph) {
        fprintf(stderr, "Failed to create graph\n");
        exit(EXIT_FAILURE);
    }

    // Генерация графа в зависимости от типа алгоритма
    if (alg_type == DIJKSTRA_LIST || alg_type == BELMAN_FORD_LIST) {
        cfg->density = density_dir(density, num_v);
        create_rand_dir_graph(cfg->graph, cfg->density, cfg->start_vertex);
        set_rand_weights_dir(cfg->graph, 1, MAX_WEIGHT);
    }
    else if (alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST || 
             alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        cfg->density = density_dir(density, num_v);
        create_rand_dir_graph(cfg->graph, cfg->density, cfg->start_vertex);
        set_rand_weights_dir(cfg->graph, 1, MAX_WEIGHT);
        cfg->inc_matrix_dir = create_inc_dir_matrix(cfg->graph, cfg->density);        
    }
    else if (alg_type == PRIM_LIST || alg_type == KRUSKAL_LIST) {
        cfg->density = density_undir(density, num_v);
        create_rand_undir_graph(cfg->graph, cfg->density);
        set_rand_weights_undir(cfg->graph, 1, MAX_WEIGHT);
    }
    else if (alg_type == PRIM_MATRIX || alg_type == KRUSKAL_MATRIX) {
        cfg->density = density_undir(density, num_v);
        create_rand_undir_graph(cfg->graph, cfg->density);
        set_rand_weights_undir(cfg->graph, 1, MAX_WEIGHT);
        cfg->inc_matrix_undir = create_inc_undir_matrix(cfg->graph, cfg->density); 
    }
}

// Создание конфигурации из существующего графа
void create_config_from_graph(Config *cfg, enum Alg_type alg_type, 
                             U32f num_v, U32f density) {
    if (alg_type < 0 || alg_type >= ALG_TYPE_COUNT) {
        fprintf(stderr, "Unknown algorithm type\n");
        exit(EXIT_FAILURE);
    }
    
    cfg->inc_matrix_dir = NULL;
    cfg->inc_matrix_undir = NULL;
    cfg->num_v = num_v;
    cfg->density = density;
    cfg->start_vertex = rand() % num_v;
    cfg->alg_type = alg_type;

    if (alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST || 
        alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST) {
        cfg->inc_matrix_dir = create_inc_dir_matrix(cfg->graph, cfg->density);        
    }
    else if (alg_type == PRIM_MATRIX || alg_type == KRUSKAL_MATRIX) {
        cfg->inc_matrix_undir = create_inc_undir_matrix(cfg->graph, cfg->density); 
    }
}

// Освобождение неиспользуемых ресурсов
void free_unused_config(Config *cfg, enum Alg_type alg_type) {
    if ((alg_type == DIJKSTRA_MATRIX || alg_type == BELMAN_FORD_MATRIX_EDGE_LIST || 
         alg_type == BELMAN_FORD_MATRIX_NO_EDGE_LIST || alg_type == PRIM_MATRIX || 
         alg_type == KRUSKAL_MATRIX) && cfg->graph) {
        free_graph(cfg->graph);
        cfg->graph = NULL;
    }
}

// Освобождение конфигурации
void free_config(Config *cfg) {
    if (!cfg) return;

    // Освобождаем граф
    if (cfg->graph) {
        free_graph(cfg->graph);
        cfg->graph = nullptr;
    }

    // Освобождаем матрицы
    if (cfg->inc_matrix_dir) {
        free(cfg->inc_matrix_dir);
        cfg->inc_matrix_dir = nullptr;
    }
    if (cfg->inc_matrix_undir) {
        free(cfg->inc_matrix_undir);
        cfg->inc_matrix_undir = nullptr;
    }

    // Освобождаем результаты SP
    if (cfg->res_sp) {
        if (cfg->res_sp->distances) {
            free(cfg->res_sp->distances);
            cfg->res_sp->distances = nullptr;
        }
        if (cfg->res_sp->parents) {
            free(cfg->res_sp->parents);
            cfg->res_sp->parents = nullptr;
        }
        free(cfg->res_sp);
        cfg->res_sp = nullptr;
    }

    // Освобождаем результаты Prim
    if (cfg->res_prim) {
        if (cfg->res_prim->parent_weight) {
            free(cfg->res_prim->parent_weight);
            cfg->res_prim->parent_weight = nullptr;
        }
        free(cfg->res_prim);
        cfg->res_prim = nullptr;
    }

    // Освобождаем результаты Kruskal
    if (cfg->res_kruskal) {
        if (cfg->res_kruskal->edges) {
            free(cfg->res_kruskal->edges);
            cfg->res_kruskal->edges = nullptr;
        }
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
// Вывод конфигурации
void print_config_file(File_config *cfg_file) {
    printf("==== CONFIGURATION ====\n");
    printf("Algorithm: %s\n", alg_names[cfg_file->alg_type]);
    printf("Vertices: %u\n", cfg_file->num_v);
    printf("Density: %.2f\n", cfg_file->density);
    printf("Start vertex: %u\n", cfg_file->start_vertex);
    printf("Output matrix: %s\n", cfg_file->out_matrix ? "true" : "false");
    printf("Output list: %s\n", cfg_file->out_list ? "true" : "false");
    
    if (cfg_file->file_name) {
        printf("Input file: %s\n", cfg_file->file_name);
    }
    
    printf("========================\n\n");
}
