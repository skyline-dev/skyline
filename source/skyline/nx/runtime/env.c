// Copyright 2018 plutoo
#include "skyline/nx/runtime/env.h"

#include <string.h>

#include "skyline/nx/kernel/svc.h"
#include "skyline/nx/kernel/thread.h"
#include "skyline/nx/runtime/hosversion.h"
#include "skyline/nx/sf/hipc.h"

static bool g_isNso = false;
static const char* g_loaderInfo = NULL;
static u64 g_loaderInfoSize = 0;
static Handle g_mainThreadHandle = INVALID_HANDLE;
static LoaderReturnFn g_loaderRetAddr = NULL;
static void* g_overrideHeapAddr = NULL;
static u64 g_overrideHeapSize = 0;
static void* g_overrideArgv = NULL;
static u64 g_syscallHints[2];
static Handle g_processHandle = CUR_PROCESS_HANDLE;
static char* g_nextLoadPath = NULL;
static char* g_nextLoadArgv = NULL;
static Result g_lastLoadResult = 0;
static bool g_hasRandomSeed = false;
static u64 g_randomSeed[2] = {0, 0};

extern __attribute__((weak)) u32 __nx_applet_type;

void envSetup(void* ctx, Handle main_thread, LoaderReturnFn saved_lr) {
    // Detect NSO environment.
    if (main_thread != -1) {
        g_mainThreadHandle = main_thread;
        g_isNso = true;

        // For NSO we assume kernelhax thus access to all syscalls.
        g_syscallHints[0] = g_syscallHints[1] = -1ull;

        return;
    }

    // Parse NRO config entries.
    ConfigEntry* ent = ctx;

    while (ent->Key != EntryType_EndOfList) {
        switch (ent->Key) {
            case EntryType_MainThreadHandle:
                g_mainThreadHandle = ent->Value[0];
                break;

            case EntryType_NextLoadPath:
                g_nextLoadPath = (char*)ent->Value[0];
                g_nextLoadArgv = (char*)ent->Value[1];
                break;

            case EntryType_OverrideHeap:
                g_overrideHeapAddr = (void*)ent->Value[0];
                g_overrideHeapSize = ent->Value[1];
                break;

            case EntryType_Argv:
                g_overrideArgv = (void*)ent->Value[1];
                break;

            case EntryType_SyscallAvailableHint:
                g_syscallHints[0] = ent->Value[0];
                g_syscallHints[1] = ent->Value[1];
                break;
            case EntryType_ProcessHandle:
                g_processHandle = ent->Value[0];
                break;

            case EntryType_LastLoadResult:
                g_lastLoadResult = ent->Value[0];
                break;

            case EntryType_RandomSeed:
                g_hasRandomSeed = true;
                g_randomSeed[0] = ent->Value[0];
                g_randomSeed[1] = ent->Value[1];
                break;
            case EntryType_HosVersion:
                hosversionSet(ent->Value[0]);
                break;
        }

        ent++;
    }

    g_loaderInfoSize = ent->Value[1];
    if (g_loaderInfoSize) {
        g_loaderInfo = (const char*)(uintptr_t)ent->Value[0];
    }
}
const char* envGetLoaderInfo(void) { return g_loaderInfo; }

u64 envGetLoaderInfoSize(void) { return g_loaderInfoSize; }

Handle envGetMainThreadHandle(void) { return g_mainThreadHandle; }

bool envIsNso(void) { return g_isNso; }

bool envHasHeapOverride(void) { return g_overrideHeapAddr != NULL; }

void* envGetHeapOverrideAddr(void) { return g_overrideHeapAddr; }

u64 envGetHeapOverrideSize(void) { return g_overrideHeapSize; }

bool envHasArgv(void) { return g_overrideArgv != NULL; }

void* envGetArgv(void) { return g_overrideArgv; }

bool envIsSyscallHinted(u8 svc) {
    // return !!(g_syscallHints[svc/64] & (1ull << (svc%64)));

    return true;  // assume all svc calls are avaiable
}

Handle envGetOwnProcessHandle(void) { return g_processHandle; }

void envSetOwnProcessHandle(Handle handle) { g_processHandle = handle; }

LoaderReturnFn envGetExitFuncPtr(void) { return g_loaderRetAddr; }

void envSetExitFuncPtr(LoaderReturnFn addr) { g_loaderRetAddr = addr; }

Result envSetNextLoad(const char* path, const char* argv) {
    strcpy(g_nextLoadPath, path);

    if (g_nextLoadArgv != NULL) {
        if (argv == NULL)
            g_nextLoadArgv[0] = '\0';
        else
            strcpy(g_nextLoadArgv, argv);
    }

    return 0;
}

bool envHasNextLoad(void) { return g_nextLoadPath != NULL; }

Result envGetLastLoadResult(void) { return g_lastLoadResult; }

bool envHasRandomSeed(void) { return g_hasRandomSeed; }

void envGetRandomSeed(u64 out[2]) {
    out[0] = g_randomSeed[0];
    out[1] = g_randomSeed[1];
}
