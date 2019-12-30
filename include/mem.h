/**
 * @file mem.h
 * @brief Memory functions.
 */

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* WEAK memset(void *src, int val, u64 num);
void* WEAK memcpy(void *dest, void const *src, u64 count);
void* WEAK memmove( void* dest, const void* src, u64 count);
void* WEAK memalign(size_t alignment, size_t size);
void* WEAK memmem(void* needle, size_t needleLen, void* haystack, size_t haystackLen);

#ifdef __cplusplus
}
#endif