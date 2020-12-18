/**
 * @file alloc.h
 * @brief Allocation functions.
 */

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t size);
void free(void* src);
void* calloc(u64 num, u64 size);
void* realloc(void* ptr, u64 size);
void* aligned_alloc(u64 alignment, u64 size);
u64 malloc_usable_size(void* ptr);

#ifdef __cplusplus
}
#endif