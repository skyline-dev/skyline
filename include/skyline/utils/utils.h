#pragma once

#include "types.h"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/kernel/svc.h"

u32 previousPowerOfTwo(u32 x);
Result memGetMap(MemoryInfo* info, u64 addr);
u64 memGetMapAddr(u64 addr);
u64 memNextMap(u64);
u64 memNextMapOfType(u64, u32);
u64 memNextMapOfPerm(u64, u32);
u64 get_program_id();
void inlineHandler();

#ifdef __cplusplus
};
#endif
