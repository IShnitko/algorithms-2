#include "prim.h"
#include "../../config/configuration.h"
#include "../../graph/graph.h"
#include "../../utils/min_heap.h"
#include "../../utils/timer.h"
#include <climits>
#include <cstdlib>
#include <cstdio>

// Реализация для списка смежности
void prim_list(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f start_vertex = cfg->start_vertex;
    Graph* graph = cfg->graph;

    // Инициализация результатов
    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    cfg->res_prim->parent_weight = (PrimResult*)malloc(num_v * sizeof(PrimResult));

    // Инициализация массивов
    U32f* key = (U32f*)malloc(num_v * sizeof(U32f));
    U32f* parent = (U32f*)malloc(num_v * sizeof(U32f));
    bool* inMST = (bool*)malloc(num_v * sizeof(bool));

    for (U32f i = 0; i < num_v; i++) {
        key[i] = UINT32_MAX;
        parent[i] = UINT32_MAX;
        inMST[i] = false;
    }
    key[start_vertex] = 0;
    parent[start_vertex] = start_vertex;

    // Создание и инициализация минимальной кучи
    MinHeap* heap = create_min_heap(num_v);
    min_heap_insert(heap, start_vertex, key[start_vertex]);

    // Замер времени выполнения
    Timer timer;

    // Основной алгоритм
    while (!min_heap_is_empty(heap)) {
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;
        inMST[u] = true;

        // Перебор соседей
        Node* node = graph->adjLists[u];
        while (node != nullptr) {
            U32f v = node->vertex;
            U32f weight = node->weight;

            // Если вершина v ещё не в MST и вес меньше текущего ключа
            if (!inMST[v] && weight < key[v]) {
                key[v] = weight;
                parent[v] = u;

                if (heap->indices[v] != UINT32_MAX) {
                    min_heap_decrease_key(heap, v, key[v]);
                } else {
                    min_heap_insert(heap, v, key[v]);
                }
            }
            node = node->next;
        }
    }

    // Фиксация времени выполнения
    cfg->execution_time = timer.elapsed();

    // Сохранение результатов
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_prim->parent_weight[i].parent = parent[i];
        cfg->res_prim->parent_weight[i].weight = (i == start_vertex) ? 0 : key[i];
    }

    // Освобождение ресурсов
    free_min_heap(heap);
    free(key);
    free(parent);
    free(inMST);
}
// Реализация для матрицы инцидентности
void prim_matrix(Config* cfg) {
    U32f num_v = cfg->num_v;
    U32f density = cfg->density;
    U32f start_vertex = cfg->start_vertex;
    U32f* inc_matrix = cfg->inc_matrix_undir;

    // Инициализация результатов
    cfg->res_prim = (Res_prim*)malloc(sizeof(Res_prim));
    cfg->res_prim->parent_weight = (PrimResult*)malloc(num_v * sizeof(PrimResult));

    // Инициализация массивов
    U32f* key = (U32f*)malloc(num_v * sizeof(U32f));
    U32f* parent = (U32f*)malloc(num_v * sizeof(U32f));
    bool* inMST = (bool*)malloc(num_v * sizeof(bool));

    for (U32f i = 0; i < num_v; i++) {
        key[i] = UINT32_MAX;
        parent[i] = UINT32_MAX;
        inMST[i] = false;
    }
    key[start_vertex] = 0;
    parent[start_vertex] = start_vertex;

    // Создание и инициализация минимальной кучи
    MinHeap* heap = create_min_heap(num_v);
    min_heap_insert(heap, start_vertex, key[start_vertex]);

    // Замер времени выполнения
    Timer timer;

    // Основной алгоритм
    while (!min_heap_is_empty(heap)) {
        HeapItem min_item = min_heap_extract_min(heap);
        U32f u = min_item.vertex;
        inMST[u] = true;

        // Перебор рёбер через матрицу инцидентности
        for (U32f j = 0; j < density; j++) {
            U32f idx = u * density + j;
            if (inc_matrix[idx] != 0) {
                // Поиск второй вершины в ребре
                U32f v = UINT32_MAX;
                for (U32f k = 0; k < num_v; k++) {
                    if (k == u) continue;
                    if (inc_matrix[k * density + j] != 0) {
                        v = k;
                        break;
                    }
                }

                if (v == UINT32_MAX) continue;
                U32f weight = inc_matrix[idx];

                // Если вершина v ещё не в MST и вес меньше текущего ключа
                if (!inMST[v] && weight < key[v]) {
                    key[v] = weight;
                    parent[v] = u;

                    if (heap->indices[v] != UINT32_MAX) {
                        min_heap_decrease_key(heap, v, key[v]);
                    } else {
                        min_heap_insert(heap, v, key[v]);
                    }
                }
            }
        }
    }

    // Фиксация времени выполнения
    cfg->execution_time = timer.elapsed();

    // Сохранение результатов
    for (U32f i = 0; i < num_v; i++) {
        cfg->res_prim->parent_weight[i].parent = parent[i];
        cfg->res_prim->parent_weight[i].weight = (i == start_vertex) ? 0 : key[i];
    }

    // Освобождение ресурсов
    free_min_heap(heap);
    free(key);
    free(parent);
    free(inMST);
}
