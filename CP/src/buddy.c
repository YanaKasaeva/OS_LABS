#include "buddy.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MIN_ORDER 4
#define MAX_ORDER 20

typedef struct BuddyBlock {
    struct BuddyBlock *next;
    size_t order;
    size_t user_size;
} BuddyBlock;

struct Allocator {
    BuddyBlock *free_lists[MAX_ORDER + 1];
    void *memory_start;
    size_t total_memory;
    size_t used_memory;
    size_t user_used_memory;
    size_t mmap_size;
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

Allocator* buddy_create(size_t size) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t aligned_size = ((size + sizeof(Allocator) + page_size - 1) / page_size) * page_size;
    
    void *memory = mmap(NULL, aligned_size, 
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) return NULL;
    
    Allocator *a = (Allocator*)memory;
    memset(a->free_lists, 0, sizeof(a->free_lists));
    
    a->memory_start = (char*)memory + sizeof(Allocator);
    a->total_memory = aligned_size - sizeof(Allocator);
    a->used_memory = 0;
    a->user_used_memory = 0;
    a->mmap_size = aligned_size;

    size_t max_possible = a->total_memory;
    size_t order = MIN_ORDER;
    while (order < MAX_ORDER && (1UL << (order + 1)) <= max_possible) {
        order++;
    }
    
    BuddyBlock *b = (BuddyBlock*)a->memory_start;
    b->next = NULL;
    b->order = order;
    b->user_size = 0;
    a->free_lists[order] = b;

    return a;
}

void buddy_destroy(Allocator *a) {
    if (a) {
        munmap(a, a->mmap_size);
    }
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
        buddy->user_size = 0;
        a->free_lists[i] = buddy;
    }

    block->order = order;
    block->user_size = size;
    block->next = NULL;
    
    size_t block_size = 1UL << order;
    a->used_memory += block_size;
    a->user_used_memory += size;

    return (char*)block + sizeof(BuddyBlock);
}

void buddy_free(Allocator *a, void *ptr) {
    if (!ptr || !a) return;

    BuddyBlock *block = (BuddyBlock*)((char*)ptr - sizeof(BuddyBlock));
    size_t order = block->order;
    
    a->used_memory -= (1UL << order);
    a->user_used_memory -= block->user_size;

    block->user_size = 0;

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
                block->user_size = 0;
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
    return a ? a->user_used_memory : 0;
}

size_t buddy_get_total_allocated(Allocator *a) {
    return a ? a->used_memory : 0;
}

size_t buddy_get_free_memory(Allocator *a) {
    if (!a) return 0;
    
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