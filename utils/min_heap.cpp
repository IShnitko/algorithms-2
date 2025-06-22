#include "min_heap.h"
#include <cstdlib>
#include <climits>

// Create a new MinHeap with given capacity
MinHeap* create_min_heap(U32f capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->items = (HeapItem*)malloc(capacity * sizeof(HeapItem));  // Allocate array for heap elements
    heap->indices = (U32f*)malloc(capacity * sizeof(U32f));         // Map vertex to heap index
    heap->capacity = capacity;
    heap->size = 0;

    // Initialize all indices to UINT32_MAX to mark vertices as not in heap
    for (U32f i = 0; i < capacity; i++) {
        heap->indices[i] = UINT32_MAX;
    }

    return heap;
}

// Free memory allocated for the heap
void free_min_heap(MinHeap* heap) {
    if (heap) {
        free(heap->items);
        free(heap->indices);
        free(heap);
    }
}

// Swap two HeapItem elements
void swap(HeapItem* a, HeapItem* b) {
    HeapItem temp = *a;
    *a = *b;
    *b = temp;
}

// Restore heap property moving the element at index upward
void min_heapify_up(MinHeap* heap, U32f index) {
    while (index > 0) {
        U32f parent_index = (index - 1) / 2;
        if (heap->items[index].distance < heap->items[parent_index].distance) {
            // Update indices to reflect swap
            heap->indices[heap->items[index].vertex] = parent_index;
            heap->indices[heap->items[parent_index].vertex] = index;
            swap(&heap->items[index], &heap->items[parent_index]);
            index = parent_index;
        } else {
            break;
        }
    }
}

// Restore heap property moving the element at index downward
void min_heapify_down(MinHeap* heap, U32f index) {
    while (index < heap->size) {
        U32f left = 2 * index + 1;
        U32f right = 2 * index + 2;
        U32f smallest = index;

        // Find smallest among node and its children
        if (left < heap->size && heap->items[left].distance < heap->items[smallest].distance) {
            smallest = left;
        }

        if (right < heap->size && heap->items[right].distance < heap->items[smallest].distance) {
            smallest = right;
        }

        if (smallest != index) {
            // Update indices before swapping
            heap->indices[heap->items[index].vertex] = smallest;
            heap->indices[heap->items[smallest].vertex] = index;
            swap(&heap->items[index], &heap->items[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

// Insert a new vertex with its distance into the heap
void min_heap_insert(MinHeap* heap, U32f vertex, U32f distance) {
    if (heap->size >= heap->capacity) return;  // Heap full

    heap->items[heap->size].vertex = vertex;
    heap->items[heap->size].distance = distance;
    heap->indices[vertex] = heap->size;
    min_heapify_up(heap, heap->size);
    heap->size++;
}

// Decrease the distance value of a given vertex and reheapify
void min_heap_decrease_key(MinHeap* heap, U32f vertex, U32f new_distance) {
    U32f index = heap->indices[vertex];
    if (index == UINT32_MAX || new_distance >= heap->items[index].distance) {
        // Vertex not in heap or new distance is not smaller
        return;
    }

    heap->items[index].distance = new_distance;
    min_heapify_up(heap, index);
}

// Extract the minimum distance vertex from the heap
HeapItem min_heap_extract_min(MinHeap* heap) {
    if (heap->size == 0) {
        // Return empty item if heap is empty
        HeapItem empty = {UINT32_MAX, UINT32_MAX};
        return empty;
    }

    HeapItem min_item = heap->items[0];
    heap->indices[min_item.vertex] = UINT32_MAX; // Mark vertex as removed

    heap->size--;
    if (heap->size > 0) {
        heap->items[0] = heap->items[heap->size];
        heap->indices[heap->items[0].vertex] = 0;
        min_heapify_down(heap, 0);
    }

    return min_item;
}

// Check if the heap is empty
bool min_heap_is_empty(MinHeap* heap) {
    return heap->size == 0;
}
