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
void dijkstra_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f density = cfg->density;
    U32f start_vertex = cfg->start_vertex;
    I32f* inc_matrix = cfg->inc_matrix_dir;

    // Инициализация результатов
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));

    // Инициализация массивов
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Создание и инициализация минимальной кучи
    MinHeap* heap = create_min_heap(num_v);
    min_heap_insert(heap, start_vertex, 0);

    // Замер времени выполнения
    Timer timer;
    timer.start();

    // Основной алгоритм
    while (!min_heap_is_empty(heap)) {
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;

        // Перебор рёбер через матрицу инцидентности
        for (U32f j = 0; j < density; j++) {
            I32f val = inc_matrix[u * density + j];
            if (val < 0) { // Исходящее ребро
                U32f weight = -val;

                // Поиск конечной вершины
                for (U32f v = 0; v < num_v; v++) {
                    if (v == u) continue;
                    if (inc_matrix[v * density + j] > 0) {
                        // Проверка на переполнение
                        if (cfg->res_sp->distances[u] > UINT32_MAX - weight) {
                            break;
                        }

                        U32f alt = cfg->res_sp->distances[u] + weight;

                        if (alt < cfg->res_sp->distances[v]) {
                            cfg->res_sp->distances[v] = alt;
                            cfg->res_sp->parents[v] = u;

                            if (heap->indices[v] != UINT32_MAX) {
                                min_heap_decrease_key(heap, v, alt);
                            } else {
                                min_heap_insert(heap, v, alt);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    // Фиксация времени выполнения
    cfg->execution_time = timer.stop();

    // Освобождение ресурсов
    free_min_heap(heap);
}
