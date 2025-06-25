#include "graph.h"    // Contains the Graph and Node structure definitions.
#include <stdlib.h>   // For malloc, free, NULL.
#include <stdio.h>    // For printf, fprintf.

// Creates a new Node structure for an adjacency list.
// vertex: The destination vertex of the edge.
// weight: The weight of the edge.
// Returns: A pointer to the newly created Node, or NULL if memory allocation fails.
Node* create_node(U32f vertex, U32f weight) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) { // Check if malloc was successful.
        node->vertex = vertex; // Set the destination vertex.
        node->weight = weight; // Set the edge weight.
        node->next = NULL;     // Initialize 'next' pointer to NULL.
    } else {
        fprintf(stderr, "Error: Memory allocation failed for a new node.\n");
    }
    return node;
}

// Creates a new Graph structure with a specified number of vertices.
// vertices: The number of vertices the graph will contain.
// Returns: A pointer to the newly created Graph, or NULL if memory allocation fails.
Graph* create_graph(U32f vertices) {
    printf("Creating graph with %lu vertices\n", vertices);

    // Basic validation: A graph must have at least one vertex.
    if (vertices == 0) {
        fprintf(stderr, "Error: Cannot create graph with 0 vertices.\n");
        return NULL;
    }

    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph) { // Check if malloc for Graph structure was successful.
        graph->num_v = vertices; // Set the number of vertices.
        // Allocate memory for the adjacency lists.
        // calloc initializes all allocated memory to zero (NULL pointers for Node*).
        graph->adjLists = (Node**)calloc(vertices, sizeof(Node*));
        if (!graph->adjLists) { // Check if calloc for adjLists was successful.
            fprintf(stderr, "Error: Memory allocation failed for adjacency lists.\n");
            free(graph); // Free the previously allocated graph structure.
            return NULL;
        }
    } else {
        fprintf(stderr, "Error: Memory allocation failed for graph structure.\n");
    }
    return graph;
}

// Adds a directed edge from a source vertex (src) to a destination vertex (dst) with a given weight.
// graph: Pointer to the Graph structure.
// src: The source vertex of the edge.
// dst: The destination vertex of the edge.
// weight: The weight of the edge.
void add_edge(Graph* graph, U32f src, U32f dst, U32f weight) {
    // Validate graph pointer and vertex indices.
    if (!graph || src >= graph->num_v || dst >= graph->num_v) {
        fprintf(stderr, "Error: Invalid graph pointer or vertex indices (%lu->%lu) for adding edge.\n", src, dst);
        return;
    }

    // Create a new node for the destination vertex.
    Node* node = create_node(dst, weight);
    if (node) { // If node creation was successful.
        node->next = graph->adjLists[src]; // Link the new node to the beginning of the src's adjacency list.
        graph->adjLists[src] = node;       // Update the head of src's list.
    }
    // If node creation failed (due to malloc failure), an error message is already printed by create_node.
}

// Checks if an edge exists between a source vertex (src) and a destination vertex (dst).
// graph: Pointer to the Graph structure.
// src: The source vertex.
// dst: The destination vertex.
// Returns: 1 if the edge exists, 0 otherwise.
int check_edge(Graph* graph, U32f src, U32f dst) {
    // Validate graph pointer and vertex indices.
    if (!graph || src >= graph->num_v || dst >= graph->num_v) {
        // fprintf(stderr, "Warning: Invalid graph pointer or vertex indices (%lu, %lu) for checking edge.\n", src, dst);
        return 0; // Edge does not exist if parameters are invalid.
    }

    // Traverse the adjacency list of the source vertex.
    Node* temp = graph->adjLists[src];
    while (temp) {
        if (temp->vertex == dst) return 1; // Found the edge.
        temp = temp->next;
    }
    return 0; // Edge not found.
}

// Retrieves the Node representing the edge from source (src) to destination (dst).
// graph: Pointer to the Graph structure.
// src: The source vertex.
// dst: The destination vertex.
// Returns: A pointer to the Node if the edge exists, otherwise NULL.
Node* get_edge(Graph* graph, U32f src, U32f dst) {
    // Validate graph pointer and vertex indices.
    if (!graph || src >= graph->num_v || dst >= graph->num_v) {
        // fprintf(stderr, "Warning: Invalid graph pointer or vertex indices (%lu, %lu) for getting edge.\n", src, dst);
        return NULL;
    }

    // Traverse the adjacency list of the source vertex.
    Node* temp = graph->adjLists[src];
    while (temp) {
        if (temp->vertex == dst) return temp; // Found the node representing the edge.
        temp = temp->next;
    }
    return NULL; // Edge not found.
}

// Assigns random weights to all edges in an undirected graph.
// Ensures that for an edge (u, v), the weight of (u->v) is the same as (v->u).
// graph: Pointer to the Graph structure.
// min_weight: Minimum possible weight.
// max_weight: Maximum possible weight.
void set_rand_weights_undir(Graph* graph, U32f min_weight, U32f max_weight) {
    if (!graph) {
        fprintf(stderr, "Error: Invalid graph pointer for setting random undirected weights.\n");
        return;
    }

    // Iterate through each vertex.
    for (U32f i = 0; i < graph->num_v; i++) {
        // Traverse the adjacency list of the current vertex 'i'.
        Node* temp = graph->adjLists[i];
        while (temp) {
            // For undirected graphs, we only assign a weight once for each unique edge (u,v).
            // By checking temp->vertex > i, we ensure each edge (e.g., 0-1) is processed only when i=0, not when i=1.
            if (temp->vertex > i) {
                // Generate a random weight within the specified range.
                temp->weight = min_weight + (rand() % (max_weight - min_weight + 1));
                // Find the reverse edge (temp->vertex -> i) and assign the same weight.
                Node* reverse_edge = get_edge(graph, temp->vertex, i);
                if (reverse_edge) {
                    reverse_edge->weight = temp->weight;
                } else {
                    // This indicates an inconsistency if the graph is truly undirected.
                    fprintf(stderr, "Warning: Undirected graph inconsistent, reverse edge (%lu->%lu) not found.\n", temp->vertex, i);
                }
            }
            temp = temp->next;
        }
    }
}

// Assigns random weights to all edges in a directed graph.
// graph: Pointer to the Graph structure.
// min_weight: Minimum possible weight.
// max_weight: Maximum possible weight.
void set_rand_weights_dir(Graph* graph, U32f min_weight, U32f max_weight) {
    if (!graph) {
        fprintf(stderr, "Error: Invalid graph pointer for setting random directed weights.\n");
        return;
    }

    // Iterate through each vertex.
    for (U32f i = 0; i < graph->num_v; i++) {
        // Traverse the adjacency list of the current vertex 'i'.
        Node* temp = graph->adjLists[i];
        while (temp) {
            // Generate a random weight for each directed edge independently.
            temp->weight = min_weight + (rand() % (max_weight - min_weight + 1));
            temp = temp->next;
        }
    }
}

// Frees all dynamically allocated memory associated with a Graph structure.
// This includes all nodes in all adjacency lists and the adjacency list array itself,
// then finally the Graph structure.
// graph: Pointer to the Graph structure to be freed.
void free_graph(Graph* graph) {
    if (!graph) return; // Do nothing if graph pointer is NULL.

    if (graph->adjLists) { // Check if adjacency lists array was allocated.
        // Iterate through each adjacency list.
        for (U32f i = 0; i < graph->num_v; i++) {
            Node* temp = graph->adjLists[i];
            while (temp) {
                Node* next = temp->next; // Store the pointer to the next node before freeing current.
                free(temp);              // Free the current node.
                temp = next;             // Move to the next node.
            }
        }
        free(graph->adjLists); // Free the array of adjacency list heads.
        graph->adjLists = nullptr; // Set to nullptr to avoid dangling pointer.
    }
    free(graph); // Free the Graph structure itself.
}
