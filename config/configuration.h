#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../graph/graph.h"

// Общие определения
typedef uint_fast32_t U32f;
typedef int_fast32_t I32f;

// Типы алгоритмов
enum Alg_type {
    DIJKSTRA_LIST,
    DIJKSTRA_MATRIX,
    BELMAN_FORD_LIST,
    BELMAN_FORD_MATRIX_EDGE_LIST,
    BELMAN_FORD_MATRIX_NO_EDGE_LIST,
    PRIM_LIST,
    PRIM_MATRIX,
    KRUSKAL_LIST,
    KRUSKAL_MATRIX,
    ALG_TYPE_COUNT
};

// Объявление массива имен алгоритмов
extern const char* alg_names[ALG_TYPE_COUNT];

// Результаты алгоритмов (перед Config!)
typedef struct Res_sp {
    U32f* distances;
    U32f* parents;
} Res_sp;

typedef struct PrimResult {
    U32f parent;
    U32f weight;
} PrimResult;

typedef struct KruskalEdge {
    U32f u;
    U32f v;
    U32f weight;
} KruskalEdge;

typedef struct Res_prim {
    PrimResult* parent_weight;
} Res_prim;

typedef struct Res_kruskal {
    KruskalEdge* edges;
    U32f num_edges;
} Res_kruskal;

// Основная конфигурация
typedef struct Config {
    Graph* graph;
    I32f* inc_matrix_dir;
    U32f* inc_matrix_undir;
    U32f num_v;
    U32f density;
    U32f start_vertex;
    Res_sp* res_sp;
    Res_prim* res_prim;
    Res_kruskal* res_kruskal;
    enum Alg_type alg_type;
} Config;

// Конфигурация файла
typedef struct File_config {
    char* file_name;
    U32f num_v;
    double density;
    U32f start_vertex;
    int out_matrix;
    int out_list;
    enum Alg_type alg_type;
} File_config;

// Прототипы функций
void create_config_random_weights(Config *cfg, U32f num_v, double density,
                                 enum Alg_type alg_type, U32f start_vertex);
void create_config_from_graph(Config *cfg, enum Alg_type alg_type,
                             U32f num_v, U32f density);
void free_unused_config(Config *cfg, enum Alg_type alg_type);
void free_config(Config *cfg);
void free_config_file(File_config *cfg);
void read_config_file(const char* filename, File_config* config);
void print_config_file(File_config *cfg_file);
void run_config_file_var(File_config *cfg_file, Config *cfg);
void run_config_file_load(File_config *cfg_file, Config *cfg);

#endif // CONFIGURATION_H