/**
 * @file alloc.h
 * @brief Allocation functions.
 */

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void*  WEAK malloc(size_t size);
void  WEAK free(void *src);
void*  WEAK calloc(u64 num, u64 size);
void* WEAK  realloc(void *ptr, u64 size);
void*  WEAK aligned_alloc(u64 alignment, u64 size);
u64  WEAK malloc_usable_size(void *ptr);

#ifdef __cplusplus
}
#endif