#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdlib.h>

// Типы для работы с графами
typedef uint_fast32_t U32f;
typedef int_fast32_t I32f;

// Узел списка смежности
typedef struct Node {
    U32f vertex;
    U32f weight;
    struct Node* next;
} Node;

// Структура графа (список смежности)
typedef struct {
    U32f num_v;        // Количество вершин
    Node** adjLists;   // Массив списков смежности
} Graph;

// Создание графа
Graph* create_graph(U32f vertices);

// Добавление ребра
void add_edge(Graph* graph, U32f src, U32f dst, U32f weight);

// Проверка существования ребра
int check_edge(Graph* graph, U32f src, U32f dst);

// Освобождение памяти графа
void free_graph(Graph* graph);

// Получение ребра
Node* get_edge(Graph* graph, U32f src, U32f dst);

#endif // GRAPH_H