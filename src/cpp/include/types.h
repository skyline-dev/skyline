/**
 * @file types.h
 * @brief Defines common types.
 */

#include <inttypes.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "skyline/nx/result.h"

#pragma once

#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long
#define _QWORD __int64
#define _DWORD __int32
#define _WORD __int16
#define _BYTE char

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef __int128_t s128;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;
typedef float f32;
typedef double f64;

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef u32 uint;

// stores a result on a lot of OS-related functions
typedef u32 Result;
typedef u32 Handle;
typedef void (*ThreadFunc)(void*);

#ifndef BIT
#define BIT(n) (1ULL << (n))
#endif

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
#endif

#ifndef WEAK
#define WEAK __attribute__((weak))
#endif

#ifndef ALIGNA
#define ALIGNA(x) __attribute__((aligned(x)))
#endif

#ifndef IGNORE_ARG
#define IGNORE_ARG(x) (void)(x)
#endif

#define NX_INLINE __attribute__((always_inline)) static inline

#if __cplusplus >= 201402L
#define NX_CONSTEXPR NX_INLINE constexpr
#else
#define NX_CONSTEXPR NX_INLINE
#endif

#define INVALID_HANDLE ((Handle)0)

#define PAGE_SIZE 0x1000

#ifndef ALIGN_UP
#define ALIGN_UP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#endif

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, a) ((unsigned long)(x) & ~(((unsigned long)(a)) - 1))
#endif

#define R_ERRORONFAIL(r) \
    if (R_FAILED(r)) *((Result*)0x69) = r;

#define R_UNLESS(expr, res)                  \
    ({                                       \
        if (!(expr)) {                       \
            return static_cast<Result>(res); \
        }                                    \
    })
