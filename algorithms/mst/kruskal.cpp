#include "kruskal.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/dsu.h"
#include "../../utils/timer.h"
#include <cstdlib>
#include <cstdio>

// Структура ребра для алгоритма Крускала
struct Edge {
    U32f u;
    U32f v;
    U32f weight;
};

// Функция сравнения рёбер для сортировки
int compare_edges(const void* a, const void* b) {
    Edge* edgeA = (Edge*)a;
    Edge* edgeB = (Edge*)b;
    if (edgeA->weight < edgeB->weight) return -1;
    if (edgeA->weight > edgeB->weight) return 1;
    return 0;
}

// Реализация для списка смежности
void kruskal_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    Graph* graph = cfg->graph;

    // Подсчитаем общее количество рёбер
    U32f total_edges = 0;
    for (U32f i = 0; i < num_v; i++) {
        Node* node = graph->adjLists[i];
        while (node != nullptr) {
            // Для неориентированного графа учитываем только одно направление
            if (i < node->vertex) {
                total_edges++;
            }
            node = node->next;
        }
    }

    // Создаем массив рёбер
    Edge* edges = (Edge*)malloc(total_edges * sizeof(Edge));
    U32f edge_count = 0;

    // Заполняем массив рёбер
    for (U32f i = 0; i < num_v; i++) {
        Node* node = graph->adjLists[i];
        while (node != nullptr) {
            U32f j = node->vertex;
            if (i < j) { // Чтобы не дублировать рёбра
                edges[edge_count].u = i;
                edges[edge_count].v = j;
                edges[edge_count].weight = node->weight;
                edge_count++;
            }
            node = node->next;
        }
    }

    // Сортировка рёбер по весу
    qsort(edges, total_edges, sizeof(Edge), compare_edges);

    // Инициализация DSU
    DSU dsu(num_v);

    // Инициализация результатов
    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    cfg->res_kruskal->edges = (KruskalEdge*)malloc((num_v - 1) * sizeof(KruskalEdge));
    cfg->res_kruskal->num_edges = 0;

    // Замер времени выполнения
    Timer timer;
    timer.start();

    // Основной алгоритм: перебираем рёбра в порядке возрастания веса
    for (U32f i = 0; i < total_edges; i++) {
        U32f u = edges[i].u;
        U32f v = edges[i].v;

        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v);

            // Добавляем ребро в результат
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].u = u;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].v = v;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].weight = edges[i].weight;
            cfg->res_kruskal->num_edges++;

            // Если построили MST (V-1 ребро), выходим
            if (cfg->res_kruskal->num_edges == num_v - 1) break;
        }
    }

    // Фиксация времени выполнения
    cfg->execution_time = timer.stop();

    // Освобождение ресурсов
    free(edges);
}
// Реализация для матрицы инцидентности
void kruskal_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f density = cfg->density;
    U32f* inc_matrix = cfg->inc_matrix_undir;

    // Создаем массив рёбер
    struct Edge {
        U32f u;
        U32f v;
        U32f weight;
    };

    Edge* edges = (Edge*)malloc(density * sizeof(Edge));
    U32f edge_count = 0;

    // Заполняем массив рёбер: пробегаем по всем столбцам (рёбрам)
    for (U32f j = 0; j < density; j++) {
        U32f u = UINT32_MAX;
        U32f v = UINT32_MAX;
        U32f weight = 0;

        for (U32f i = 0; i < num_v; i++) {
            if (inc_matrix[i * density + j] != 0) {
                if (u == UINT32_MAX) {
                    u = i;
                    weight = inc_matrix[i * density + j];
                } else {
                    v = i;
                    break;
                }
            }
        }

        if (u != UINT32_MAX && v != UINT32_MAX) {
            edges[edge_count].u = u;
            edges[edge_count].v = v;
            edges[edge_count].weight = weight;
            edge_count++;
        }
    }

    // Сортировка рёбер по весу
    qsort(edges, edge_count, sizeof(Edge), compare_edges);

    // Инициализация DSU
    DSU dsu(num_v);

    // Инициализация результатов
    cfg->res_kruskal = (Res_kruskal*)malloc(sizeof(Res_kruskal));
    cfg->res_kruskal->edges = (KruskalEdge*)malloc((num_v - 1) * sizeof(KruskalEdge));
    cfg->res_kruskal->num_edges = 0;

    // Замер времени выполнения
    Timer timer;
    timer.start();

    // Основной алгоритм
    for (U32f i = 0; i < edge_count; i++) {
        U32f u = edges[i].u;
        U32f v = edges[i].v;

        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v);

            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].u = u;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].v = v;
            cfg->res_kruskal->edges[cfg->res_kruskal->num_edges].weight = edges[i].weight;
            cfg->res_kruskal->num_edges++;

            if (cfg->res_kruskal->num_edges == num_v - 1) break;
        }
    }

    // Фиксация времени выполнения
    cfg->execution_time = timer.stop();

    // Освобождение ресурсов
    free(edges);
}
