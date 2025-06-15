#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <cstdint>

typedef uint_fast32_t U32f;

typedef struct {
    U32f vertex;
    U32f distance;
} HeapItem;

typedef struct {
    HeapItem* items;
    U32f size;
    U32f capacity;
    U32f* indices; // For decrease_key: vertex -> index in heap
} MinHeap;

MinHeap* create_min_heap(U32f capacity);
void free_min_heap(MinHeap* heap);
void min_heap_insert(MinHeap* heap, U32f vertex, U32f distance);
void min_heap_decrease_key(MinHeap* heap, U32f vertex, U32f new_distance);
HeapItem min_heap_extract_min(MinHeap* heap);
bool min_heap_is_empty(MinHeap* heap);

#endif // MIN_HEAP_H