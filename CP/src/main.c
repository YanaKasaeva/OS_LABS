#include "freelist.h"
#include "buddy.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MEMORY_SIZE (1024*1024)
#define N 100000

typedef struct {
    double alloc_time;
    double free_time;
    size_t used_mem;
    size_t free_mem;
} Stats;

static Stats test_allocator(const char *name,
                            Allocator *A,
                            void* (*alloc_func)(Allocator*, size_t),
                            void  (*free_func)(Allocator*, void*),
                            size_t (*get_used)(Allocator*),
                            size_t (*get_free)(Allocator*)) {
    (void)name;

    void *ptrs[N];
    size_t sizes[N];
    for (int i = 0; i < N; i++) sizes[i] = (rand() % 256) + 16;

    Stats s = {0};

    clock_t start = clock();
    for (int i = 0; i < N; i++)
        ptrs[i] = alloc_func(A, sizes[i]);
    clock_t end = clock();
    s.alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

    s.used_mem = get_used(A);
    s.free_mem = get_free(A);

    start = clock();
    for (int i = 0; i < N; i++)
        free_func(A, ptrs[i]);
    end = clock();
    s.free_time = (double)(end - start) / CLOCKS_PER_SEC;

    return s;
}

int main(void) {
    srand((unsigned)time(NULL));

    void *mem_ff = malloc(MEMORY_SIZE);
    void *mem_bd = malloc(MEMORY_SIZE);

    Allocator *ff = freelist_create(mem_ff, MEMORY_SIZE);
    Allocator *bd = buddy_create(mem_bd, MEMORY_SIZE);

    Stats s_ff = test_allocator("Free List", ff, freelist_alloc, freelist_free,
                                freelist_get_used_memory, freelist_get_free_memory);
    Stats s_bd = test_allocator("Buddy", bd, buddy_alloc, buddy_free,
                                buddy_get_used_memory, buddy_get_free_memory);

    printf("=== Сравнение аллокаторов ===\n");
    printf("%-10s | %-11s | %-10s | %-10s\n",
           "Allocator", "Utilization", "Alloc Time", "Free Time");
    printf("--------------------------------------------------\n");

    printf("%-10s | %-10.2f%% | %-10.6f | %-10.6f\n",
           "Free List",
           100.0 * s_ff.used_mem / (s_ff.used_mem + s_ff.free_mem),
           s_ff.alloc_time,
           s_ff.free_time);

    printf("%-10s | %-10.2f%% | %-10.6f | %-10.6f\n",
           "Buddy",
           100.0 * s_bd.used_mem / (s_bd.used_mem + s_bd.free_mem),
           s_bd.alloc_time,
           s_bd.free_time);

    free(mem_ff);
    free(mem_bd);

    return 0;
}