#include "configuration.h"
#include "../graph/generators.h"
#include "../io/display.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

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
void read_config_file(const char* filename, File_config* config) {
    FILE* file = fopen(filename, "r"); 
    if (!file) {
        fprintf(stderr, "Cannot open config file: %s\n", filename);
        exit(1);
    }
    
    char section[128];
    char value[128];
    
    while (fscanf(file, "%127s", section) == 1) {
        if (section[0] == '.') {
            if (fscanf(file, "%127s", value) != 1) continue;
            
            if (strcmp(section, ".alg_type") == 0) {
                parse_alg_type(value, config);
            }
            else if (strcmp(section, ".num_v") == 0) {
                parse_num_v(value, config);
            }
            else if (strcmp(section, ".density") == 0) {
                parse_density(value, config);
            }
            else if (strcmp(section, ".start_vertex") == 0) {
                parse_start_vertex(value, config);
            }
            else if (strcmp(section, ".out_matrix") == 0) {
                parse_out_flag(value, &config->out_matrix);
            }
            else if (strcmp(section, ".out_list") == 0) {
                parse_out_flag(value, &config->out_list);
            }
            else if (strcmp(section, ".file_name") == 0) {
                parse_file_name(value, config);
            }
        }
    }
    
    fclose(file);
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
    if (cfg->graph) {
        free_graph(cfg->graph);
    }
    if (cfg->inc_matrix_dir) {
        free(cfg->inc_matrix_dir);
    }
    if (cfg->inc_matrix_undir) {
        free(cfg->inc_matrix_undir);
    }
    if (cfg->res_sp) {
        free(cfg->res_sp->distances);
        free(cfg->res_sp->parents);
        free(cfg->res_sp);
    }
}

// Освобождение конфигурации файла
void free_config_file(File_config *cfg) {
    if (cfg->file_name) {
        free(cfg->file_name);
    }
    free(cfg);
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
