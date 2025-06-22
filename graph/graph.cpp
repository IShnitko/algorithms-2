#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

// Create a new adjacency list node
Node* create_node(U32f vertex, U32f weight) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->vertex = vertex;
        node->weight = weight;
        node->next = NULL;
    }
    return node;
}

// Create a graph with a given number of vertices
Graph* create_graph(U32f vertices) {
    printf("Creating graph with %u vertices\n", vertices);

    if (vertices == 0) {
        fprintf(stderr, "Error: Cannot create graph with 0 vertices\n");
        return NULL;
    }

    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph) {
        graph->num_v = vertices;
        graph->adjLists = (Node**)calloc(vertices, sizeof(Node*));  // Initialize adjacency list
        if (!graph->adjLists) {
            free(graph);
            return NULL;
        }
    }
    return graph;
}

// Add a directed edge from src to dst with a weight
void add_edge(Graph* graph, U32f src, U32f dst, U32f weight) {
    if (!graph || src >= graph->num_v || dst >= graph->num_v) return;

    Node* node = create_node(dst, weight);
    if (node) {
        node->next = graph->adjLists[src];
        graph->adjLists[src] = node;
    }
}

// Check if there is an edge from src to dst
int check_edge(Graph* graph, U32f src, U32f dst) {
    if (!graph || src >= graph->num_v || dst >= graph->num_v) return 0;

    Node* temp = graph->adjLists[src];
    while (temp) {
        if (temp->vertex == dst) return 1;
        temp = temp->next;
    }
    return 0;
}

// Get the edge (node) from src to dst, if it exists
Node* get_edge(Graph* graph, U32f src, U32f dst) {
    if (!graph || src >= graph->num_v || dst >= graph->num_v) return NULL;

    Node* temp = graph->adjLists[src];
    while (temp) {
        if (temp->vertex == dst) return temp;
        temp = temp->next;
    }
    return NULL;
}

// Assign random weights to all undirected edges in the graph
void set_rand_weights_undir(Graph* graph, U32f min_weight, U32f max_weight) {
    if (!graph) return;

    for (U32f i = 0; i < graph->num_v; i++) {
        Node* temp = graph->adjLists[i];
        while (temp) {
            // Set weight only for one direction to avoid double assignment
            if (temp->vertex > i) {
                temp->weight = min_weight + (rand() % (max_weight - min_weight + 1));
                Node* reverse_edge = get_edge(graph, temp->vertex, i);
                if (reverse_edge) {
                    reverse_edge->weight = temp->weight;
                }
            }
            temp = temp->next;
        }
    }
}

// Assign random weights to all directed edges
void set_rand_weights_dir(Graph* graph, U32f min_weight, U32f max_weight) {
    if (!graph) return;

    for (U32f i = 0; i < graph->num_v; i++) {
        Node* temp = graph->adjLists[i];
        while (temp) {
            temp->weight = min_weight + (rand() % (max_weight - min_weight + 1));
            temp = temp->next;
        }
    }
}

// Free all memory used by the graph
void free_graph(Graph* graph) {
    if (!graph) return;

    if (graph->adjLists) {
        for (U32f i = 0; i < graph->num_v; i++) {
            Node* temp = graph->adjLists[i];
            while (temp) {
                Node* next = temp->next;
                free(temp);
                temp = next;
            }
        }
        free(graph->adjLists);
        graph->adjLists = NULL;
    }
    free(graph);
}
