#include "../sp/dijkstra.h"
#include "../../config/configuration.h"
#include "../../utils/min_heap.h"
#include "../../utils/timer.h"
#include <cstdlib>
#include <climits>
#include <cstdio>

void dijkstra_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f start_vertex = cfg->start_vertex;

    // Initialize results
    cfg->res_sp = (Res_sp*)malloc(sizeof(Res_sp));
    cfg->res_sp->distances = (U32f*)malloc(num_v * sizeof(U32f));
    cfg->res_sp->parents = (U32f*)malloc(num_v * sizeof(U32f));

    // Initialize arrays
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_sp->distances[i] = UINT32_MAX;
        cfg->res_sp->parents[i] = UINT32_MAX;
    }
    cfg->res_sp->distances[start_vertex] = 0;

    // Create and initialize min-heap
    MinHeap* heap = create_min_heap(num_v);
    min_heap_insert(heap, start_vertex, 0);

    // Timer start
    Timer timer;
    timer.start();

    // Main algorithm
    while (!min_heap_is_empty(heap)) {
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;

        // Explore neighbors
        Node* node = cfg->graph->adjLists[u];
        while (node != nullptr) {
            U32f v = node->vertex;
            U32f weight = node->weight;
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
            node = node->next;
        }
    }

    // Timer stop and save
    cfg->execution_time = timer.stop();

    // Cleanup
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
