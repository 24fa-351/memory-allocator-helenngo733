#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef SYSTEM_MALLOC
#define mod_malloc malloc
#define mod_free free
#define mod_realloc realloc
#else
#include "malloc.h"
#endif

// int rand_between(int min, int max) { return rand() % (max - min + 1) + min; }

#define FILL_PATTERN 0xAA  // Test pattern to fill memory

void fill_memory(void *ptr, size_t size, unsigned char pattern) { memset(ptr, pattern, size); }

int verify_memory(void *ptr, size_t size, unsigned char pattern) {
    unsigned char *mem = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        if (mem[i] != pattern) {
            return 0;  // Memory is not correctly filled
        }
    }
    return 1;  // Memory is correctly filled
}

// Test 1
void test_small_allocations() {
    printf("Running test_small_allocations...\n");
    char *ptr1 = (char *)mod_malloc(16);
    char *ptr2 = (char *)mod_malloc(32);

    if (!ptr1 || !ptr2) {
        printf("Failed: Memory allocation failed.\n");
        return;
    }

    fill_memory(ptr1, 16, FILL_PATTERN);
    fill_memory(ptr2, 32, FILL_PATTERN);

    if (!verify_memory(ptr1, 16, FILL_PATTERN) || !verify_memory(ptr2, 32, FILL_PATTERN)) {
        printf("Failed: Memory integrity check failed.\n");
        mod_free(ptr1);
        mod_free(ptr2);
        return;
    }

    mod_free(ptr1);
    mod_free(ptr2);

    printf("Passed: Small allocation test.\n\n");
}

// Test 2
void test_large_allocation() {
    printf("Running test_large_allocation...\n");
    char *ptr = (char *)mod_malloc(1024 * 1024);  // 1 MB

    if (!ptr) {
        printf("Failed: Memory allocation failed.\n");
        return;
    }

    fill_memory(ptr, 1024 * 1024, FILL_PATTERN);

    if (!verify_memory(ptr, 1024 * 1024, FILL_PATTERN)) {
        printf("Failed: Memory integrity check failed.\n");
        mod_free(ptr);
        return;
    }

    mod_free(ptr);
    printf("Passed: Large allocation test.\n\n");
}

// Test 3
void test_alloc_free_sequence() {
    printf("Running test_alloc_free_sequence...\n");
    char *ptr1 = (char *)mod_malloc(64);
    char *ptr2 = (char *)mod_malloc(128);
    char *ptr3 = (char *)mod_malloc(256);

    if (!ptr1 || !ptr2 || !ptr3) {
        printf("Failed: Memory allocation failed.\n");
        return;
    }

    fill_memory(ptr1, 64, FILL_PATTERN);
    fill_memory(ptr2, 128, FILL_PATTERN);
    fill_memory(ptr3, 256, FILL_PATTERN);

    if (!verify_memory(ptr1, 64, FILL_PATTERN) || !verify_memory(ptr2, 128, FILL_PATTERN) ||
        !verify_memory(ptr3, 256, FILL_PATTERN)) {
        printf("Failed: Memory integrity check failed.\n");
        mod_free(ptr1);
        mod_free(ptr2);
        mod_free(ptr3);
        return;
    }

    mod_free(ptr1);
    mod_free(ptr2);
    mod_free(ptr3);

    printf("Passed: Allocate and free sequence test.\n\n");
}

// Test 4
void test_realloc_behavior() {
    printf("Running test_realloc_behavior...\n");
    char *ptr = (char *)mod_malloc(64);

    if (!ptr) {
        printf("Failed: Memory allocation failed.\n");
        return;
    }

    fill_memory(ptr, 64, FILL_PATTERN);

    if (!verify_memory(ptr, 64, FILL_PATTERN)) {
        printf("Failed: Memory integrity check failed.\n");
        mod_free(ptr);
        return;
    }

    // Reallocate with increased size
    ptr = (char *)mod_realloc(ptr, 128);
    if (!ptr) {
        printf("Failed: Memory reallocation failed.\n");
        return;
    }

    fill_memory(ptr, 128, FILL_PATTERN);

    if (!verify_memory(ptr, 128, FILL_PATTERN)) {
        printf("Failed: Memory integrity check failed after realloc.\n");
        mod_free(ptr);
        return;
    }

    mod_free(ptr);
    printf("Passed: Reallocation behavior test.\n\n");
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 't') {
        int test_num = atoi(argv[2]);

        switch (test_num) {
            case 1:
                test_small_allocations();
                break;
            case 2:
                test_large_allocation();
                break;
            case 3:
                test_alloc_free_sequence();
                break;
            case 4:
                test_realloc_behavior();
                break;
        }
    } else {
        test_small_allocations();
        test_alloc_free_sequence();
        test_realloc_behavior();
        test_large_allocation();
    }

    return 0;
}
