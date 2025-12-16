#include "freelist.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

struct Allocator {
    Block *free_list;
    size_t total_memory;
    size_t used_memory;
};

Allocator* freelist_create(void *memory, size_t size) {
    Allocator *a = (Allocator*)memory;
    a->total_memory = size - sizeof(Allocator);
    a->used_memory = 0;

    a->free_list = (Block*)((char*)memory + sizeof(Allocator));
    a->free_list->size = a->total_memory;
    a->free_list->next = NULL;
    return a;
}

void* freelist_alloc(Allocator *a, size_t size) {
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
    if (!ptr) return;
    Block *b = (Block*)((char*)ptr - sizeof(Block));
    a->used_memory -= b->size + sizeof(Block);
    b->next = a->free_list;
    a->free_list = b;
}

size_t freelist_get_used_memory(Allocator *a) {
    return a->used_memory;
}

size_t freelist_get_free_memory(Allocator *a) {
    return a->total_memory - a->used_memory;
}