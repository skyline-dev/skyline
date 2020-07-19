#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nnosMutexType {
    u8 curState;            // _0
    bool isRecursiveMutex;  // _1
    s32 lockLevel;          // _2
    u8 _6[0x20 - 0xE];
} nnosMutexType;

void nnosInitializeMutex(nnosMutexType*, bool, s32);
void nnosFinalizeMutex(nnosMutexType*);
void nnosLockMutex(nnosMutexType*);
bool nnosTryLockMutex(nnosMutexType*);
void nnosUnlockMutex(nnosMutexType*);

long long int llabs(long long int n);

#ifdef __cplusplus
}
#endif