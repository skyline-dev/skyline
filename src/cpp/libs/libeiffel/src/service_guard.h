// Copyright 2017-2020 libnx Authors

// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.

// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <switch.h>

typedef struct ServiceGuard {
    Mutex mutex;
    u32 refCount;
} ServiceGuard;

NX_INLINE bool serviceGuardBeginInit(ServiceGuard* g) {
    mutexLock(&g->mutex);
    return (g->refCount++) == 0;
}

NX_INLINE Result serviceGuardEndInit(ServiceGuard* g, Result rc, void (*cleanupFunc)(void)) {
    if (R_FAILED(rc)) {
        cleanupFunc();
        --g->refCount;
    }
    mutexUnlock(&g->mutex);
    return rc;
}

NX_INLINE void serviceGuardExit(ServiceGuard* g, void (*cleanupFunc)(void)) {
    mutexLock(&g->mutex);
    if (g->refCount && (--g->refCount) == 0) cleanupFunc();
    mutexUnlock(&g->mutex);
}

#define NX_GENERATE_SERVICE_GUARD_PARAMS(name, _paramdecl, _parampass)                    \
                                                                                          \
    static ServiceGuard g_##name##Guard;                                                  \
    NX_INLINE Result _##name##Initialize _paramdecl;                                      \
    static void _##name##Cleanup(void);                                                   \
                                                                                          \
    Result name##Initialize _paramdecl {                                                  \
        Result rc = 0;                                                                    \
        if (serviceGuardBeginInit(&g_##name##Guard)) rc = _##name##Initialize _parampass; \
        return serviceGuardEndInit(&g_##name##Guard, rc, _##name##Cleanup);               \
    }                                                                                     \
                                                                                          \
    void name##Exit(void) { serviceGuardExit(&g_##name##Guard, _##name##Cleanup); }

#define NX_GENERATE_SERVICE_GUARD(name) NX_GENERATE_SERVICE_GUARD_PARAMS(name, (void), ())
