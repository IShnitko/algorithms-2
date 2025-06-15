#include "generators.h"
#include <stdlib.h>
#include <stdio.h>

// Внутренние функции (не экспортируются)
static U32f* prufer_to_tree(U32f* seq, U32f len_seq);
static U32f* generate_prufer_seq(U32f vertices);
static void shuffle_edges(U32f* arr_u, U32f* arr_v, size_t n);

void generate_undirected_graph(Graph* graph, double density) {
    U32f vertices = graph->num_v;
    
    // Генерация остовного дерева
    U32f* seq = generate_prufer_seq(vertices - 2);
    U32f* edges = prufer_to_tree(seq, vertices - 2);
    free(seq);
    
    for (U32f i = 0; i < vertices - 1; i++) {
        add_edge(graph, edges[2*i], edges[2*i+1], 0);
        add_edge(graph, edges[2*i+1], edges[2*i], 0);
    }
    free(edges);
    
    // Добавление дополнительных ребер
    U32f max_edges = vertices * (vertices - 1) / 2;
    U32f target_edges = (U32f)(max_edges * density);
    U32f additional_edges = target_edges - (vertices - 1);
    
    if (additional_edges <= 0) return;
    
    // Создание кандидатов для добавления
    U32f* candidates_u = malloc(max_edges * sizeof(U32f));
    U32f* candidates_v = malloc(max_edges * sizeof(U32f));
    U32f count = 0;
    
    for (U32f i = 0; i < vertices; i++) {
        for (U32f j = i + 1; j < vertices; j++) {
            if (!check_edge(graph, i, j)) {
                candidates_u[count] = i;
                candidates_v[count] = j;
                count++;
            }
        }
    }
    
    shuffle_edges(candidates_u, candidates_v, count);
    
    U32f to_add = (additional_edges < count) ? additional_edges : count;
    for (U32f i = 0; i < to_add; i++) {
        add_edge(graph, candidates_u[i], candidates_v[i], 0);
        add_edge(graph, candidates_v[i], candidates_u[i], 0);
    }
    
    free(candidates_u);
    free(candidates_v);
}

// Реализация других функций аналогично с адаптацией кода