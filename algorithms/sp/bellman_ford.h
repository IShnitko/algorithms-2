#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include "../../config/configuration.h"

void bellman_ford_list(Config* cfg);
void bellman_ford_matrix_edge_list(Config* cfg);
void bellman_ford_matrix_no_edge_list(Config* cfg);

#endif // BELLMAN_FORD_H