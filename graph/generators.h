#ifndef GENERATORS_H
#define GENERATORS_H

#include "graph.h"

void create_rand_undir_graph(Graph *graph, I32f density);
void create_rand_dir_graph(Graph *graph, I32f density, U32f start_vertex);
U32f *create_inc_undir_matrix(Graph* graph, U32f density);
I32f *create_inc_dir_matrix(Graph* graph, U32f density);
Graph* generate_connected_graph(U32f vertices, double density, bool directed);

#endif // GENERATORS_H