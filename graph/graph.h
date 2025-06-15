#ifndef GRAPH_H
#define GRAPH_H

#include <inttypes.h>

typedef uint_fast32_t U32f;
typedef int_fast32_t I32f;

typedef struct Node {
    U32f vertex;
    U32f weight;
    struct Node* next;
} Node;

typedef struct {
    U32f num_v;
    Node** adjLists;
} Graph;

Graph* create_graph(U32f vertices);
void add_edge(Graph* graph, U32f src, U32f dst, U32f weight);
int check_edge(Graph* graph, U32f src, U32f dst);
void free_graph(Graph* graph);
Node* get_edge(Graph* graph, U32f src, U32f dst);
void set_rand_weights_undir(Graph* graph, U32f min_weight, U32f max_weight);
void set_rand_weights_dir(Graph* graph, U32f min_weight, U32f max_weight);

#endif // GRAPH_H