#include "generators.h"
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

static void swap(U32f *a, U32f *b) {
    U32f temp = *a;
    *a = *b;
    *b = temp;
}

static void shuffle(U32f *arr_u, U32f *arr_v, size_t n) {
    if (n <= 1) return;
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        swap(&arr_u[i], &arr_u[j]);
        swap(&arr_v[i], &arr_v[j]);
    }
}

static U32f* prufer_to_tree(U32f *seq, U32f len_seq) {
    U32f *edges = (U32f *)malloc(sizeof(U32f) * 2 * (len_seq + 1));
    int i_edges = 0;
    U32f length = len_seq + 2;
    U32f *degree = (U32f *)malloc(length * sizeof(U32f));

    for (U32f i = 0; i < length; i++) {
        degree[i] = 1;
    }

    for (U32f i = 0; i < len_seq; i++) {
        U32f node = seq[i];
        degree[node]++;
    }

    for (U32f i = 0; i < len_seq; i++) {
        U32f node_seq = seq[i];
        for (U32f j = 0; j < length; j++) {
            if (degree[j] == 1) {
                edges[i_edges * 2] = node_seq;
                edges[i_edges * 2 + 1] = j;
                i_edges++;
                degree[j]--;
                degree[node_seq]--;
                break;
            }
        }
    }

    U32f u = 0, v = 0;
    int first = 1;
    for (U32f i = 0; i < length; i++) {
        if (degree[i] == 1) {
            if (first) {
                u = i;
                first = 0;
            } else {
                v = i;
            }
        }
    }
    edges[i_edges * 2] = u;
    edges[i_edges * 2 + 1] = v;

    free(degree);
    return edges;
}

static U32f *prufer_rand_seq(U32f verticies) {
    U32f* seq = (U32f *)malloc(sizeof(U32f) * verticies);
    for (U32f i = 0; i < verticies; i++) {
        seq[i] = rand() % verticies;
    }
    return seq;
}

void create_rand_undir_graph(Graph *graph, I32f density) {
    U32f verticies = graph->num_v;
    U32f *seq = prufer_rand_seq(verticies - 2);
    U32f *edges = prufer_to_tree(seq, verticies - 2);
    free(seq);

    for (U32f i = 0; i < verticies - 1; i++) {
        add_edge(graph, edges[i * 2], edges[i * 2 + 1], 0);
        add_edge(graph, edges[i * 2 + 1], edges[i * 2], 0);
    }

    free(edges);
    U32f max_edges = verticies * (verticies - 1) / 2;
    U32f numb = max_edges - verticies + 1;

    U32f* candidates_u = (U32f *)malloc(sizeof(U32f) * numb);
    U32f* candidates_v = (U32f *)malloc(sizeof(U32f) * numb);
    U32f ind = 0;

    for (U32f i = 0; i < verticies; i++) {
        for (U32f j = i + 1; j < verticies; j++) {
            if (check_edge(graph, i, j)) continue;
            candidates_u[ind] = i;
            candidates_v[ind] = j;
            ind++;
        }
    }

    shuffle(candidates_u, candidates_v, ind);
    density = density - (verticies - 1);

    if (density > ind) density = ind;
    else if (density < 0) density = 0;

    for (U32f i = 0; i < density; i++) {
        add_edge(graph, candidates_u[i], candidates_v[i], 0);
        add_edge(graph, candidates_v[i], candidates_u[i], 0);
    }

    free(candidates_u);
    free(candidates_v);
}

static void get_dir_tree(Graph *graph, U32f *parent, U32f start_vertex, U32f *visited) {
    visited[start_vertex] = 1;
    for (Node *temp = graph->adjLists[start_vertex]; temp != NULL; temp = temp->next) {
        if (!visited[temp->vertex]) {
            parent[temp->vertex] = start_vertex;
            get_dir_tree(graph, parent, temp->vertex, visited);
        }
    }
}

void create_rand_dir_graph(Graph *graph, I32f density, U32f start_vertex) {
    U32f verticies = graph->num_v;
    U32f *seq = prufer_rand_seq(verticies - 2);
    U32f *edges = prufer_to_tree(seq, verticies - 2);
    free(seq);

    Graph *t_graph = create_graph(verticies);
    for (U32f i = 0; i < verticies - 1; i++) {
        add_edge(t_graph, edges[i * 2], edges[i * 2 + 1], 0);
        add_edge(t_graph, edges[i * 2 + 1], edges[i * 2], 0);
    }
    free(edges);

    U32f *parent = (U32f *)malloc(verticies * sizeof(U32f));
    U32f *visited = (U32f *)calloc(verticies, sizeof(U32f));
    get_dir_tree(t_graph, parent, start_vertex, visited);

    free(visited);
    free_graph(t_graph);

    for (U32f i = 0; i < verticies; i++) {
        if (i != start_vertex) {
            add_edge(graph, parent[i], i, 0);
        }
    }
    free(parent);

    U32f max_edges = verticies * (verticies - 1);
    U32f numb = max_edges - verticies + 1;

    U32f *candidates_u = (U32f *)malloc(sizeof(U32f) * numb);
    U32f *candidates_v = (U32f *)malloc(sizeof(U32f) * numb);
    I32f ind = 0;

    for (U32f i = 0; i < verticies; i++) {
        for (U32f j = 0; j < verticies; j++) {
            if (i == j || check_edge(graph, i, j)) continue;
            candidates_u[ind] = i;
            candidates_v[ind] = j;
            ind++;
        }
    }

    shuffle(candidates_u, candidates_v, ind);
    density = density - (verticies - 1);

    if (ind > 0 && density > ind) density = ind;
    else if (density < 0) density = 0;

    for (U32f i = 0; i < density; i++) {
        add_edge(graph, candidates_u[i], candidates_v[i], 0);
    }

    free(candidates_u);
    free(candidates_v);
}

U32f *create_inc_undir_matrix(Graph* graph, U32f density) {

    printf("Creating undirected incidence matrix: vertices=%u, density=%u\n", graph->num_v, density);
    U32f *inc_matrix = (U32f *)calloc(graph->num_v * density, sizeof(U32f));
    if (!inc_matrix) {
        fprintf(stderr, "Memory allocation failed for incidence matrix\n");
        return NULL;
    }

    U32f ind = 0;
    for (U32f i = 0; i < graph->num_v; i++) {
        Node* temp = graph->adjLists[i];
        while (temp) {
            U32f j = temp->vertex;
            if (i < j) {
                inc_matrix[i * density + ind] = temp->weight;
                inc_matrix[j * density + ind] = temp->weight;
                ind++;
            }
            temp = temp->next;
        }
    }
    return inc_matrix;
}

I32f *create_inc_dir_matrix(Graph* graph, U32f density) {
    printf("Creating directed incidence matrix: vertices=%u, density=%u\n", graph->num_v, density);
    I32f *inc_matrix = (I32f *)calloc(graph->num_v * density, sizeof(I32f));
    if (!inc_matrix) {
        fprintf(stderr, "Memory allocation failed for incidence matrix\n");
        return NULL;
    }

    U32f ind = 0;
    for (U32f i = 0; i < graph->num_v; i++) {
        Node* temp = graph->adjLists[i];
        while (temp) {
            U32f j = temp->vertex;
            if (i != j) {
                inc_matrix[i * density + ind] = -temp->weight;
                inc_matrix[j * density + ind] = temp->weight;
                ind++;
            }
            temp = temp->next;
        }
    }
    return inc_matrix;
}
