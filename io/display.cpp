#include "display.h" // Include the header file for display functions.
#include <stdio.h>   // Required for printf.
#include <inttypes.h> // Required for PRIuFAST32 and PRIdFAST32 macros for printing U32f and I32f.

// Prints the adjacency list representation of a graph.
// graph: A pointer to the Graph structure to be printed.
void print_graph(Graph* graph) {
    // Check if the graph pointer is NULL to prevent dereferencing errors.
    if (!graph) return;

    // Iterate through each vertex in the graph.
    for (U32f v = 0; v < graph->num_v; v++) {
        // Print the current vertex number. PRIuFAST32 is a macro for printing uint_fast32_t.
        printf("%" PRIuFAST32 ":", v);

        // Traverse the adjacency list for the current vertex.
        Node* temp = graph->adjLists[v];
        while (temp) {
            // Print the connected vertex and the weight of the edge.
            // PRIuFAST32 is used for unsigned 32-bit fast integer.
            printf("-> (%" PRIuFAST32 ",%5" PRIuFAST32 ") ", temp->vertex, temp->weight);
            temp = temp->next; // Move to the next node in the list.
        }
        printf("\n"); // Move to the next line after printing all neighbors of the current vertex.
    }
}

// Prints an undirected incidence matrix.
// inc_matrix: A pointer to the U32f array representing the incidence matrix.
// vertices: The number of vertices (rows) in the matrix.
// density: The number of edges (columns) in the matrix.
void print_inc_undir_matrix(U32f *inc_matrix, U32f vertices, U32f density) {
    // Iterate through each row (vertex).
    for (U32f i = 0; i < vertices; i++) {
        // Iterate through each column (edge).
        for (U32f j = 0; j < density; j++) {
            // Print the element at the current position, formatted to take 5 characters.
            // PRIuFAST32 is used for unsigned 32-bit fast integer.
            printf("%5" PRIuFAST32 "", inc_matrix[i * density + j]);
        }
        printf("\n"); // Move to the next line after printing all columns for the current row.
    }
}

// Prints a directed incidence matrix.
// inc_matrix: A pointer to the I32f array representing the incidence matrix.
// vertices: The number of vertices (rows) in the matrix.
// density: The number of edges (columns) in the matrix.
void print_inc_dir_matrix(I32f *inc_matrix, U32f vertices, U32f density) {
    // Iterate through each row (vertex).
    for (U32f i = 0; i < vertices; i++) {
        // Iterate through each column (edge).
        for (U32f j = 0; j < density; j++) {
            // Print the element at the current position, formatted to take 5 characters.
            // PRIdFAST32 is used for signed 32-bit fast integer.
            printf("%5" PRIdFAST32, inc_matrix[i * density + j]);
        }
        printf("\n"); // Move to the next line after printing all columns for the current row.
    }
}
