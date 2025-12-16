#ifndef FREELIST_H
#define FREELIST_H

#include "allocator.h"

Allocator* freelist_create(void *memory, size_t size);

void* freelist_alloc(Allocator *a, size_t size);
void  freelist_free(Allocator *a, void *ptr);

size_t freelist_get_used_memory(Allocator *a);
size_t freelist_get_free_memory(Allocator *a);

#endif