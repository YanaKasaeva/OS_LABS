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
    Allocator *a = (Allocator*)memory;
    memset(a->free_lists, 0, sizeof(a->free_lists));
    a->memory_start = memory;
    a->total_memory = size - sizeof(Allocator);
    a->used_memory = 0;

    size_t order = size_to_order(a->total_memory);
    BuddyBlock *b = (BuddyBlock*)((char*)memory + sizeof(Allocator));
    b->next = NULL;
    b->order = order;
    a->free_lists[order] = b;

    return a;
}

void* buddy_alloc(Allocator *a, size_t size) {
    size_t order = size_to_order(size);
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
    if (!ptr) return;

    BuddyBlock *block = (BuddyBlock*)((char*)ptr - sizeof(BuddyBlock));
    size_t order = block->order;

    a->used_memory -= (1UL << order);

    while (order < MAX_ORDER) {
        BuddyBlock **list = &a->free_lists[order];
        BuddyBlock *prev = NULL;
        BuddyBlock *cur = *list;
        BuddyBlock *buddy = get_buddy(a->memory_start, block);
        int merged = 0;

        while (cur) {
            if (cur == buddy) {
                if (prev) prev->next = cur->next;
                else *list = cur->next;

                if ((uintptr_t)block > (uintptr_t)cur) block = cur;

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
    return a->total_memory - a->used_memory;
}