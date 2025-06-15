#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

Graph* create_graph(U32f vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) return NULL;
    
    graph->num_v = vertices;
    graph->adjLists = (Node**)calloc(vertices, sizeof(Node*));
    
    if (!graph->adjLists) {
        free(graph);
        return NULL;
    }
    return graph;
}

void add_edge(Graph* graph, U32f src, U32f dst, U32f weight) {
    if (!graph || src >= graph->num_v || dst >= graph->num_v) return;
    
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return;
    
    node->vertex = dst;
    node->weight = weight;
    node->next = graph->adjLists[src];
    graph->adjLists[src] = node;
}

int check_edge(Graph* graph, U32f src, U32f dst) {
    if (!graph || src >= graph->num_v || dst >= graph->num_v) return 0;
    
    Node* temp = graph->adjLists[src];
    while (temp) {
        if (temp->vertex == dst) return 1;
        temp = temp->next;
    }
    return 0;
}

void free_graph(Graph* graph) {
    if (!graph) return;
    
    for (U32f i = 0; i < graph->num_v; i++) {
        Node* temp = graph->adjLists[i];
        while (temp) {
            Node* next = temp->next;
            free(temp);
            temp = next;
        }
    }
    free(graph->adjLists);
    free(graph);
}

Node* get_edge(Graph* graph, U32f src, U32f dst) {
    if (!graph || src >= graph->num_v || dst >= graph->num_v) return NULL;
    
    Node* temp = graph->adjLists[src];
    while (temp) {
        if (temp->vertex == dst) return temp;
        temp = temp->next;
    }
    return NULL;
}