#ifndef DISPLAY_H
#define DISPLAY_H

#include "../graph/graph.h"
#include <inttypes.h>

void print_graph(Graph* graph);
void print_inc_undir_matrix(U32f *inc_matrix, U32f vertices, U32f density);
void print_inc_dir_matrix(I32f *inc_matrix, U32f vertices, U32f density);

#endif // DISPLAY_H