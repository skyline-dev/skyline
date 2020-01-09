#pragma once

#include "types.h"

#include "skyline/nx/kernel/svc.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef __cplusplus
extern "C" {
#endif

    u32 previousPowerOfTwo(u32 x);
    u64 memNextMap(u64);
    u64 memNextMapOfType(u64 , u32);
    u64 memNextMapOfPerm(u64 , u32);

#ifdef __cplusplus
};
#endif