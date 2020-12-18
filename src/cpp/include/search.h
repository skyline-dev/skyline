#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*));

#ifdef __cplusplus
}
#endif