#include "../sp/dijkstra.h"
#include "../../config/configuration.h"
#include "../../utils/min_heap.h"
#include "../../utils/timer.h"
#include <cstdlib>
#include <climits>
#include <cstdio>

#include "dijkstra.h"
#include "../../utils/min_heap.h"
#include <cstdio>
#include <climits>

void dijkstra_list(Config *cfg) {
    if (!cfg || !cfg->graph) {
        fprintf(stderr, "Invalid configuration for Dijkstra\n");
        return;
    }

    const U32f n = cfg->graph->num_v;
    const U32f start = cfg->start_vertex;

    // Проверка корректности стартовой вершины
    if (start >= n) {
        fprintf(stderr, "Start vertex %u is out of range [0, %u]\n", start, n-1);
        return;
    }

    // Выделяем память для результатов
    if (!cfg->res_sp) {
        cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
        cfg->res_sp->distances = (U32f*)malloc(n * sizeof(U32f));
        cfg->res_sp->parents = (U32f*)malloc(n * sizeof(U32f));
    }

    // Инициализация
    for (U32f i = 0; i < n; i++) {
        cfg->res_sp->distances[i] = UINT_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX; // Используем MAX для "не определено"
    }
    cfg->res_sp->distances[start] = 0;

    // Создаем min-heap
    MinHeap* heap = create_min_heap(n);
    min_heap_insert(heap, start, 0);

    // Основной цикл
    while (!min_heap_is_empty(heap)) {
        HeapItem item = min_heap_extract_min(heap);
        U32f u = item.vertex;

        // Перебираем соседей
        Node* neighbor = cfg->graph->adjLists[u];
        while (neighbor != nullptr) {
            U32f v = neighbor->vertex;
            U32f weight = neighbor->weight;

            // Проверка на переполнение
            if (cfg->res_sp->distances[u] > UINT_MAX - weight) {
                neighbor = neighbor->next;
                continue;
            }

            U32f new_dist = cfg->res_sp->distances[u] + weight;

            if (new_dist < cfg->res_sp->distances[v]) {
                cfg->res_sp->distances[v] = new_dist;
                cfg->res_sp->parents[v] = u;

                if (heap->indices[v] != UINT32_MAX) {
                    // Вершина уже в куче - уменьшаем ключ
                    min_heap_decrease_key(heap, v, new_dist);
                } else {
                    // Вершины нет в куче - добавляем
                    min_heap_insert(heap, v, new_dist);
                }
            }

            neighbor = neighbor->next;
        }
    }

    // Освобождаем кучу
    free_min_heap(heap);
}
void dijkstra_matrix(Config *cfg) {
    if (!cfg || !cfg->inc_matrix_dir) return;

    const U32f n = cfg->num_v;
    const U32f m = cfg->density;
    const U32f start = cfg->start_vertex;

    // Инициализация расстояний
    U32f* dist = (U32f*)malloc(n * sizeof(U32f));
    U32f* prev = (U32f*)malloc(n * sizeof(U32f));
    bool* visited = (bool*)calloc(n, sizeof(bool));

    for (U32f i = 0; i < n; i++) {
        dist[i] = UINT_MAX;
        prev[i] = UINT32_MAX;
    }
    dist[start] = 0;

    // Основной цикл
    for (U32f count = 0; count < n - 1; count++) {
        // Находим вершину с минимальным расстоянием
        U32f u = UINT32_MAX;
        U32f min_dist = UINT_MAX;
        for (U32f i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        if (u == UINT32_MAX) break;
        visited[u] = true;

        // Обновляем расстояния до соседей
        for (U32f v = 0; v < n; v++) {
            if (visited[v]) continue;

            // Ищем ребро u->v в матрице
            for (U32f e = 0; e < m; e++) {
                if (cfg->inc_matrix_dir[u * m + e] > 0 &&
                    cfg->inc_matrix_dir[v * m + e] < 0) {

                    U32f weight = cfg->inc_matrix_dir[u * m + e];
                    U32f new_dist = dist[u] + weight;

                    if (new_dist < dist[v]) {
                        dist[v] = new_dist;
                        prev[v] = u;
                    }
                    }
            }
        }
    }

    // Сохраняем результаты
    if (!cfg->res_sp) cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = dist;
    cfg->res_sp->parents = prev;
    free(visited);
}
