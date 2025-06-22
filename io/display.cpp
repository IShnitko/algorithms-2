#include "display.h"
#include <stdio.h>

// Print the adjacency list representation of the graph
void print_graph(Graph* graph) {
    if (!graph) return;

    for (U32f v = 0; v < graph->num_v; v++) {
        // Print the source vertex
        printf("%" PRIuFAST32 ":", v);
        Node* temp = graph->adjLists[v];

        // Traverse and print all adjacent vertices and their weights
        while (temp) {
            printf("-> (%" PRIuFAST32 ",%5" PRIuFAST32 ") ", temp->vertex, temp->weight);
            temp = temp->next;
        }
        printf("\n");
    }
}

// Print an incidence matrix for an undirected graph
// inc_matrix is a 1D array of size [vertices x density]
void print_inc_undir_matrix(U32f *inc_matrix, U32f vertices, U32f density) {
    for (U32f i = 0; i < vertices; i++) {
        for (U32f j = 0; j < density; j++) {
            printf("%5" PRIuFAST32 "", inc_matrix[i * density + j]);
        }
        printf("\n");
    }
}

// Print an incidence matrix for a directed graph
// Values in the matrix can be -1 (edge leaves), 1 (edge enters), or 0 (not incident)
void print_inc_dir_matrix(I32f *inc_matrix, U32f vertices, U32f density) {
    for (U32f i = 0; i < vertices; i++) {
        for (U32f j = 0; j < density; j++) {
            printf("%5" PRIdFAST32, inc_matrix[i * density + j]);
        }
        printf("\n");
    }
}
