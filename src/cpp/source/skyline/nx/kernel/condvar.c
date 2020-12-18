// Copyright 2018 plutoo
#include "skyline/nx/kernel/condvar.h"

#include "../internal.h"
#include "skyline/nx/kernel/svc.h"
#include "types.h"

Result condvarWaitTimeout(CondVar* c, Mutex* m, u64 timeout) {
    Result rc;

    rc = svcWaitProcessWideKeyAtomic((u32*)m, c, getThreadVars()->handle, timeout);

    // On timeout, we need to acquire it manually.
    if (rc == 0xEA01) mutexLock(m);

    return rc;
}