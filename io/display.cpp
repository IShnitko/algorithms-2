#include "display.h"
#include <stdio.h>

void print_graph(Graph* graph) {
    if (!graph) return;

    for (U32f v = 0; v < graph->num_v; v++) {
        printf("%" PRIuFAST32 ":", v);
        Node* temp = graph->adjLists[v];
        while (temp) {
            printf("-> (%" PRIuFAST32 ",%5" PRIuFAST32 ") ", temp->vertex, temp->weight);
            temp = temp->next;
        }
        printf("\n");
    }
}

void print_inc_undir_matrix(U32f *inc_matrix, U32f vertices, U32f density) {
    for (U32f i = 0; i < vertices; i++) {
        for (U32f j = 0; j < density; j++) {
            printf("%5" PRIuFAST32 "", inc_matrix[i * density + j]);
        }
        printf("\n");
    }
}

void print_inc_dir_matrix(I32f *inc_matrix, U32f vertices, U32f density) {
    for (U32f i = 0; i < vertices; i++) {
        for (U32f j = 0; j < density; j++) {
            printf("%5" PRIdFAST32, inc_matrix[i * density + j]);
        }
        printf("\n");
    }
}
