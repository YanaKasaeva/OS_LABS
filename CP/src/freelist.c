#include "freelist.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

struct Allocator {
    Block *free_list;
    size_t total_memory;
    size_t used_memory;
    size_t mmap_size;
};

Allocator* freelist_create(size_t size) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t aligned_size = ((size + sizeof(Allocator) + page_size - 1) / page_size) * page_size;
    
    void *memory = mmap(NULL, aligned_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) return NULL;
    
    Allocator *a = (Allocator*)memory;
    a->total_memory = aligned_size - sizeof(Allocator);
    a->used_memory = 0;
    a->mmap_size = aligned_size;

    a->free_list = (Block*)((char*)memory + sizeof(Allocator));
    a->free_list->size = a->total_memory;
    a->free_list->next = NULL;
    return a;
}

void freelist_destroy(Allocator *a) {
    if (a) {
        munmap(a, a->mmap_size);
    }
}

void* freelist_alloc(Allocator *a, size_t size) {
    if (!a || size == 0) return NULL;
    
    Block *prev = NULL;
    Block *cur = a->free_list;

    while (cur) {
        if (cur->size >= size + sizeof(Block)) {
            Block *next = (Block*)((char*)cur + sizeof(Block) + size);
            next->size = cur->size - size - sizeof(Block);
            next->next = cur->next;

            if (prev) prev->next = next;
            else a->free_list = next;

            cur->size = size;
            a->used_memory += size + sizeof(Block);
            return (char*)cur + sizeof(Block);
        }
        prev = cur;
        cur = cur->next;
    }
    return NULL;
}

void freelist_free(Allocator *a, void *ptr) {
    if (!a || !ptr) return;
    
    Block *b = (Block*)((char*)ptr - sizeof(Block));
    a->used_memory -= b->size + sizeof(Block);
    b->next = a->free_list;
    a->free_list = b;
}

size_t freelist_get_used_memory(Allocator *a) {
    return a ? a->used_memory : 0;
}

size_t freelist_get_free_memory(Allocator *a) {
    return a ? a->total_memory - a->used_memory : 0;
}