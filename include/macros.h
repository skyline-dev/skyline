/**
 * @file macros.h
 * @brief Various macros defined from IDA Pro.
 */

#pragma once

#include "types.h"

// https://github.com/joxeankoret/tahh/blob/master/comodo/defs.h

#define __CASSERT_N0__(l) COMPILE_TIME_ASSERT_##l
#define __CASSERT_N1__(l) __CASSERT_N0__(l)
#define CASSERT(cnd) typedef char __CASSERT_N1__(__LINE__)[(cnd) ? 1 : -1]

#define LODWORD(x) (*((u32*)&(x)))

template <class T>
bool is_mul_ok(T count, T elsize) {
    CASSERT((T)(-1) > 0);
    if (elsize == 0 || count == 0) return true;
    return count <= ((T)(-1)) / elsize;
}