#ifndef GENERATORS_H
#define GENERATORS_H

#include "graph.h"
#include "../utils/random.h"

// Генерация случайного неориентированного графа
void generate_undirected_graph(Graph* graph, double density);

// Генерация случайного ориентированного графа
void generate_directed_graph(Graph* graph, double density, U32f start_vertex);

// Преобразование в матрицу инцидентности (неориентированный)
U32f* create_incidence_matrix_undir(Graph* graph, U32f num_edges);

// Преобразование в матрицу инцидентности (ориентированный)
I32f* create_incidence_matrix_dir(Graph* graph, U32f num_edges);

#endif // GENERATORS_H