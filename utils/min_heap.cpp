#include "min_heap.h" // Include the header file defining MinHeap and HeapItem structures.
#include <cstdlib>    // Required for malloc and free.
#include <climits>    // Required for UINT32_MAX.

// Function to create and initialize a new MinHeap.
// capacity: The maximum number of elements the heap can hold.
// Returns: A pointer to the newly created MinHeap, or NULL if memory allocation fails.
MinHeap* create_min_heap(U32f capacity) {
    // Allocate memory for the MinHeap structure itself.
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (heap == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for MinHeap structure.\n");
        return NULL;
    }

    // Allocate memory for the array of HeapItem (the actual heap elements).
    heap->items = (HeapItem*)malloc(capacity * sizeof(HeapItem));
    if (heap->items == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for HeapItem array.\n");
        free(heap); // Free the heap structure if item allocation fails.
        return NULL;
    }

    // Allocate memory for the 'indices' array. This array maps vertex IDs to their
    // current positions (indices) in the 'items' array.
    heap->indices = (U32f*)malloc(capacity * sizeof(U32f));
    if (heap->indices == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for indices array.\n");
        free(heap->items); // Free items array.
        free(heap);         // Free heap structure.
        return NULL;
    }

    heap->capacity = capacity; // Set the maximum capacity of the heap.
    heap->size = 0;           // Initialize the current number of elements in the heap to 0.

    // Initialize all entries in the 'indices' array to UINT32_MAX.
    // This indicates that initially, no vertex is present in the heap.
    for (U32f i = 0; i < capacity; i++) {
        heap->indices[i] = UINT32_MAX;
    }

    return heap; // Return the pointer to the initialized heap.
}

// Function to free all dynamically allocated memory associated with a MinHeap.
// heap: A pointer to the MinHeap to be freed.
void free_min_heap(MinHeap* heap) {
    if (heap) { // Check if the heap pointer is not NULL.
        free(heap->items);   // Free the array of heap items.
        free(heap->indices); // Free the array of indices.
        free(heap);          // Free the MinHeap structure itself.
    }
}

// Helper function to swap two HeapItem structures.
// a: Pointer to the first HeapItem.
// b: Pointer to the second HeapItem.
void swap(HeapItem* a, HeapItem* b) {
    HeapItem temp = *a; // Store content of 'a' in a temporary variable.
    *a = *b;           // Copy content of 'b' to 'a'.
    *b = temp;         // Copy content of temporary variable to 'b'.
}

// Performs the "heapify-up" operation to restore the min-heap property after an insertion
// or a decrease-key operation. Moves an element up the heap if its distance is smaller
// than its parent's distance.
// heap: A pointer to the MinHeap.
// index: The index of the element that might violate the min-heap property.
void min_heapify_up(MinHeap* heap, U32f index) {
    // Continue as long as the current element is not the root (index > 0).
    while (index > 0) {
        U32f parent_index = (index - 1) / 2; // Calculate the parent's index.
        // If the current element's distance is less than its parent's distance.
        if (heap->items[index].distance < heap->items[parent_index].distance) {
            // Update the 'indices' array to reflect the upcoming swap.
            // The vertex at 'index' will move to 'parent_index'.
            heap->indices[heap->items[index].vertex] = parent_index;
            // The vertex at 'parent_index' will move to 'index'.
            heap->indices[heap->items[parent_index].vertex] = index;

            // Swap the HeapItem structures themselves in the 'items' array.
            swap(&heap->items[index], &heap->items[parent_index]);

            // Update 'index' to the parent's position to continue heapifying up.
            index = parent_index;
        } else {
            // If min-heap property is satisfied, stop.
            break;
        }
    }
}

// Performs the "heapify-down" operation to restore the min-heap property after an
// extraction (extract_min) or a decrease-key that potentially moved an element to the root.
// Moves an element down the heap if it's larger than one of its children.
// heap: A pointer to the MinHeap.
// index: The index of the element that might violate the min-heap property.
void min_heapify_down(MinHeap* heap, U32f index) {
    // Continue as long as the current index is within the heap bounds.
    while (index < heap->size) {
        U32f left = 2 * index + 1;  // Calculate index of left child.
        U32f right = 2 * index + 2; // Calculate index of right child.
        U32f smallest = index;      // Assume current element is the smallest.

        // Check if left child exists and has a smaller distance than 'smallest'.
        if (left < heap->size && heap->items[left].distance < heap->items[smallest].distance) {
            smallest = left; // Left child is the new smallest.
        }

        // Check if right child exists and has a smaller distance than 'smallest'.
        if (right < heap->size && heap->items[right].distance < heap->items[smallest].distance) {
            smallest = right; // Right child is the new smallest.
        }

        // If the smallest is not the current index, a swap is needed.
        if (smallest != index) {
            // Update the 'indices' array to reflect the upcoming swap.
            // The vertex at 'index' will move to 'smallest'.
            heap->indices[heap->items[index].vertex] = smallest;
            // The vertex at 'smallest' will move to 'index'.
            heap->indices[heap->items[smallest].vertex] = index;

            // Swap the HeapItem structures.
            swap(&heap->items[index], &heap->items[smallest]);

            // Update 'index' to the new position to continue heapifying down.
            index = smallest;
        } else {
            // If current element is the smallest among itself and its children, stop.
            break;
        }
    }
}

// Inserts a new vertex and its associated distance into the min-heap.
// heap: A pointer to the MinHeap.
// vertex: The ID of the vertex to insert.
// distance: The distance associated with the vertex (its priority in the heap).
void min_heap_insert(MinHeap* heap, U32f vertex, U32f distance) {
    // Check if the heap is full. If so, insertion is not possible.
    if (heap->size >= heap->capacity) {
        fprintf(stderr, "Warning: MinHeap is full. Cannot insert vertex %lu.\n", vertex);
        return;
    }

    // Place the new item at the end of the heap array.
    heap->items[heap->size].vertex = vertex;
    heap->items[heap->size].distance = distance;

    // Update the 'indices' array to store the position of the new vertex.
    heap->indices[vertex] = heap->size;

    // Restore the min-heap property by heapifying up from the newly inserted element's position.
    min_heapify_up(heap, heap->size);

    heap->size++; // Increment the size of the heap.
}

// Decreases the distance (priority) of a given vertex in the heap.
// This operation is crucial for algorithms like Dijkstra's.
// heap: A pointer to the MinHeap.
// vertex: The ID of the vertex whose distance needs to be decreased.
// new_distance: The new (smaller) distance for the vertex.
void min_heap_decrease_key(MinHeap* heap, U32f vertex, U32f new_distance) {
    // Get the current index of the vertex in the heap.
    U32f index = heap->indices[vertex];

    // Check for invalid index (vertex not in heap) or if the new distance is not actually smaller.
    if (index == UINT32_MAX || new_distance >= heap->items[index].distance) {
        // If index is UINT32_MAX, vertex is not in heap.
        // If new_distance is not less than current distance, no decrease is needed.
        return;
    }

    // Update the distance of the item at the found index.
    heap->items[index].distance = new_distance;

    // Restore the min-heap property by heapifying up from the updated element's position.
    min_heapify_up(heap, index);
}

// Extracts the item with the minimum distance (the root of the heap).
// heap: A pointer to the MinHeap.
// Returns: The HeapItem with the minimum distance. If the heap is empty, returns
//          a special HeapItem with UINT32_MAX for both vertex and distance.
HeapItem min_heap_extract_min(MinHeap* heap) {
    // If the heap is empty, return an "empty" item.
    if (heap->size == 0) {
        HeapItem empty = {UINT32_MAX, UINT32_MAX};
        return empty;
    }

    // The minimum item is always at the root (index 0).
    HeapItem min_item = heap->items[0];

    // Mark the extracted vertex as no longer in the heap in the 'indices' array.
    heap->indices[min_item.vertex] = UINT32_MAX;

    heap->size--; // Decrease the size of the heap.

    // If there are still elements in the heap after extraction.
    if (heap->size > 0) {
        // Move the last element of the heap to the root position.
        heap->items[0] = heap->items[heap->size];
        // Update the 'indices' array for the element that just moved to the root.
        heap->indices[heap->items[0].vertex] = 0;
        // Restore the min-heap property by heapifying down from the root.
        min_heapify_down(heap, 0);
    }

    return min_item; // Return the extracted minimum item.
}

// Checks if the min-heap is empty.
// heap: A pointer to the MinHeap.
// Returns: true if the heap is empty (size is 0), false otherwise.
bool min_heap_is_empty(MinHeap* heap) {
    return heap->size == 0;
}
