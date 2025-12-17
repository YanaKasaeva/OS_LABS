#include "buddy.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MIN_ORDER 4
#define MAX_ORDER 20

typedef struct BuddyBlock {
    struct BuddyBlock *next;
    size_t order;
} BuddyBlock;

struct Allocator {
    BuddyBlock *free_lists[MAX_ORDER + 1];
    void *memory_start;
    size_t total_memory;
    size_t used_memory;
};

static size_t size_to_order(size_t size) {
    size_t order = MIN_ORDER;
    size_t block_size = 1UL << order;
    while (block_size < size + sizeof(BuddyBlock)) {
        if (order >= MAX_ORDER) return 0;
        block_size <<= 1;
        order++;
    }
    return order;
}

static uintptr_t get_offset(void *base, void *ptr) {
    return (uintptr_t)ptr - (uintptr_t)base;
}

static BuddyBlock* get_buddy(void *base, BuddyBlock *block) {
    uintptr_t offset = get_offset(base, block);
    uintptr_t buddy_offset = offset ^ (1UL << block->order);
    return (BuddyBlock*)((char*)base + buddy_offset);
}

Allocator* buddy_create(void *memory, size_t size) {
    if (size < (1UL << MIN_ORDER) + sizeof(Allocator)) return NULL;
    
    Allocator *a = (Allocator*)memory;
    memset(a->free_lists, 0, sizeof(a->free_lists));
    
    // ИСПРАВЛЕНИЕ 1: memory_start указывает на начало пользовательской памяти
    a->memory_start = (char*)memory + sizeof(Allocator);
    
    // ИСПРАВЛЕНИЕ 2: total_memory - только пользовательская память
    a->total_memory = size - sizeof(Allocator);
    
    // ИСПРАВЛЕНИЕ 3: used_memory начинается с 0
    a->used_memory = 0;

    size_t max_possible = a->total_memory;
    size_t order = MIN_ORDER;
    while (order < MAX_ORDER && (1UL << (order + 1)) <= max_possible) {
        order++;
    }
    
    BuddyBlock *b = (BuddyBlock*)a->memory_start;
    b->next = NULL;
    b->order = order;
    a->free_lists[order] = b;

    return a;
}

void* buddy_alloc(Allocator *a, size_t size) {
    size_t order = size_to_order(size);
    if (order == 0) return NULL;
    
    size_t i = order;

    while (i <= MAX_ORDER && a->free_lists[i] == NULL) i++;
    if (i > MAX_ORDER) return NULL;

    BuddyBlock *block = a->free_lists[i];
    a->free_lists[i] = block->next;

    while (i > order) {
        i--;
        BuddyBlock *buddy = (BuddyBlock*)((char*)block + (1UL << i));
        buddy->next = a->free_lists[i];
        buddy->order = i;
        a->free_lists[i] = buddy;
    }

    block->order = order;
    block->next = NULL;
    a->used_memory += (1UL << order);

    return (char*)block + sizeof(BuddyBlock);
}

void buddy_free(Allocator *a, void *ptr) {
    if (!ptr || !a) return;

    BuddyBlock *block = (BuddyBlock*)((char*)ptr - sizeof(BuddyBlock));
    size_t order = block->order;

    a->used_memory -= (1UL << order);

    while (order < MAX_ORDER) {
        BuddyBlock *buddy = get_buddy(a->memory_start, block);
        BuddyBlock **list = &a->free_lists[order];
        BuddyBlock *cur = *list;
        BuddyBlock *prev = NULL;
        int merged = 0;

        while (cur) {
            if (cur == buddy) {
                if (prev) prev->next = cur->next;
                else *list = cur->next;
                
                if ((uintptr_t)block > (uintptr_t)buddy) {
                    block = buddy;
                }
                
                order++;
                block->order = order;
                merged = 1;
                break;
            }
            prev = cur;
            cur = cur->next;
        }
        if (!merged) break;
    }

    block->next = a->free_lists[block->order];
    a->free_lists[block->order] = block;
}

size_t buddy_get_used_memory(Allocator *a) {
    return a->used_memory;
}

size_t buddy_get_free_memory(Allocator *a) {
    size_t free_mem = 0;
    for (int i = MIN_ORDER; i <= MAX_ORDER; i++) {
        BuddyBlock *block = a->free_lists[i];
        while (block) {
            free_mem += (1UL << i);
            block = block->next;
        }
    }
    return free_mem;
}