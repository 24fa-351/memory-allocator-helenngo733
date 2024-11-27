#include "malloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ALIGNMENT 8
#define INITIAL_HEAP_SIZE (1024 * 1024)
#define META_SIZE (sizeof(Block))
#define MAX_AVAILABLE_BLOCKS 2048

// A block of memory on the heap
typedef struct Block {
    size_t size; //excluding the metadata
    char* ptr_start;
} Block;

// Min-heap to manage free memory blocks; needed for final version
typedef struct MinHeap {
    Block* heap[MAX_AVAILABLE_BLOCKS];
    size_t current_size;
} MinHeap;

static MinHeap free_heap;
static size_t remaining_heap_space = 0;
static void* heap_start = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void insert_into_heap(MinHeap* heap, Block* block) {
    if (heap->current_size >= MAX_AVAILABLE_BLOCKS) {
        fprintf(stderr, "Heap overflow\n");
        return;
    }

    size_t ix = heap->current_size++;
    while (ix > 0) {
        size_t parent = (ix - 1) / 2;
        if (heap->heap[parent]->size <= block->size) break;

        heap->heap[ix] = heap->heap[parent];
        ix = parent;
    }

    heap->heap[ix] = block;
}

static Block* remove_smallest_from_heap(MinHeap* heap) {
    if (heap->current_size == 0) return NULL;

    Block* min_block = heap->heap[0];
    Block* last_block = heap->heap[--heap->current_size];

    size_t ix = 0;
    while (1) {
        size_t smallest = ix;
        size_t left_child = 2 * ix + 1;
        size_t right_child = 2 * ix + 2;

        if (left_child < heap->current_size &&
            heap->heap[left_child]->size < heap->heap[smallest]->size) {
            smallest = left_child;
        }
        if (right_child < heap->current_size &&
            heap->heap[right_child]->size < heap->heap[smallest]->size) {
            smallest = right_child;
        }

        if (smallest == ix) break;

        heap->heap[ix] = heap->heap[smallest];
        ix = smallest;
    }

    heap->heap[ix] = last_block;
    return min_block;
}

// Acquiring a Block of Memory
static void* get_heap_blocks(ssize_t size) {
    void* ptr = sbrk(size);
    if (ptr == (void*)-1) return NULL;
    return ptr;
}

void* mod_malloc(size_t requested_size) {
    pthread_mutex_lock(&lock);
    
    requested_size = ((requested_size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1)) + META_SIZE;

    // Find the smallest block that can accommodate the requested size
    Block* block = remove_smallest_from_heap(&free_heap);
    while (block != NULL) {
        if (block->size >= requested_size) {
            // Split the block if there is enough space
            if (block->size > requested_size + META_SIZE) {
                Block* leftover = (Block*)((char*)block + requested_size + META_SIZE);
                leftover->size = block->size - requested_size - META_SIZE;
                insert_into_heap(&free_heap, leftover);
            }

            block->size = requested_size;
            
            pthread_mutex_unlock(&lock);
            
            memset((void*)block->ptr_start, 0, requested_size); //memory zeroed 
            return (void*)block->ptr_start;
        }
        block = remove_smallest_from_heap(&free_heap);
    }

    // If no suitable block is found, get more memory
    if (!heap_start) {
        pthread_mutex_unlock(&lock);
        
        heap_start = get_heap_blocks(INITIAL_HEAP_SIZE);
        if (!heap_start) return NULL;
        remaining_heap_space = INITIAL_HEAP_SIZE;
    }

    while (remaining_heap_space < requested_size + META_SIZE) {
        void* new_heap_start = get_heap_blocks(INITIAL_HEAP_SIZE);
        if (!new_heap_start) return NULL;
        remaining_heap_space += INITIAL_HEAP_SIZE;
    }

    // Create a new block from the remaining heap space
    Block* new_block = (Block*)heap_start;
    new_block->size = requested_size;
    new_block->ptr_start = (char*)heap_start + META_SIZE;
    heap_start = (void*)((char*)heap_start + requested_size + META_SIZE);
    remaining_heap_space -= requested_size + META_SIZE;

    pthread_mutex_unlock(&lock);
    memset((void*)new_block->ptr_start, 0, requested_size);
    return (void*)new_block->ptr_start;
}

void mod_free(void* ptr) {
    if (!ptr) return;
    pthread_mutex_lock(&lock);
    
    Block* block = (Block*)((char*)ptr - META_SIZE);
    block->ptr_start = (char*)ptr;
    insert_into_heap(&free_heap, block);
    
    pthread_mutex_unlock(&lock);
}

void* mod_realloc(void* ptr, size_t new_size) {
    if (!ptr) return mod_malloc(new_size);
    if (new_size == 0) {
        mod_free(ptr);
        return NULL;
    }

    Block* block = (Block*)((char*)ptr - META_SIZE);
    if (block->size >= new_size) return ptr;

    void* new_ptr = mod_malloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        mod_free(ptr);
    }
    return new_ptr;
}
