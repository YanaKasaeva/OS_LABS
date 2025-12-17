#include "freelist.h"
#include "buddy.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEMORY_SIZE (1024*1024)
#define N 10000

typedef struct {
    double alloc_time;
    double free_time;
    size_t used_mem;
    size_t free_mem;
    int successful_allocs;
} Stats;

static Stats test_allocator(const char *name,
                            Allocator *A,
                            void* (*alloc_func)(Allocator*, size_t),
                            void  (*free_func)(Allocator*, void*),
                            size_t (*get_used)(Allocator*),
                            size_t (*get_free)(Allocator*)) {
    void *ptrs[N] = {0};
    size_t sizes[N];
    int successful_allocs = 0;
    
    for (int i = 0; i < N; i++) {
        sizes[i] = (rand() % 112) + 16;
    }

    Stats s = {0};

    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        ptrs[i] = alloc_func(A, sizes[i]);
        if (ptrs[i]) successful_allocs++;
    }
    clock_t end = clock();
    s.alloc_time = (double)(end - start) / CLOCKS_PER_SEC;
    s.successful_allocs = successful_allocs;
    
    s.used_mem = get_used(A);
    s.free_mem = get_free(A);

    start = clock();
    for (int i = 0; i < N; i++) {
        if (ptrs[i]) {
            free_func(A, ptrs[i]);
        }
    }
    end = clock();
    s.free_time = (double)(end - start) / CLOCKS_PER_SEC;

    return s;
}

int main(void) {
    srand((unsigned)time(NULL));

    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t aligned_size = ((MEMORY_SIZE + page_size - 1) / page_size) * page_size;
    
    void *mem_ff = mmap(NULL, aligned_size, 
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *mem_bd = mmap(NULL, aligned_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (mem_ff == MAP_FAILED || mem_bd == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    Allocator *ff = freelist_create(mem_ff, aligned_size);
    Allocator *bd = buddy_create(mem_bd, aligned_size);

    Stats s_ff = test_allocator("Free List", ff, freelist_alloc, freelist_free,
                                freelist_get_used_memory, freelist_get_free_memory);
    Stats s_bd = test_allocator("Buddy", bd, buddy_alloc, buddy_free,
                                buddy_get_used_memory, buddy_get_free_memory);

    printf("%-10s | %-11s | %-12s | %-12s\n",
           "Allocator", "Utilization", "Alloc Time", "Free Time");
    printf("-----------------------------------------------------\n");

    printf("%-10s | %-10.2f%% | %-12.6f | %-12.6f\n",
           "Free List",
           100.0 * s_ff.used_mem / aligned_size,
           s_ff.alloc_time,
           s_ff.free_time);

    printf("%-10s | %-10.2f%% | %-12.6f | %-12.6f\n",
           "Buddy",
           100.0 * s_bd.used_mem / aligned_size,
           s_bd.alloc_time,
           s_bd.free_time);

    munmap(mem_ff, aligned_size);
    munmap(mem_bd, aligned_size);

    return 0;
}