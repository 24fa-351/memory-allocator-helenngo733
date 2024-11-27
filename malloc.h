#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

void* mod_malloc(size_t size);
void mod_free(void* ptr);
void* mod_realloc(void* ptr, size_t size);

#endif