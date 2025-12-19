#ifndef BUDDY_H
#define BUDDY_H

#include "allocator.h"

Allocator* buddy_create(size_t size);
void buddy_destroy(Allocator *a);

void* buddy_alloc(Allocator *a, size_t size);
void  buddy_free(Allocator *a, void *ptr);

size_t buddy_get_used_memory(Allocator *a);
size_t buddy_get_total_allocated(Allocator *a);
size_t buddy_get_free_memory(Allocator *a);

#endif