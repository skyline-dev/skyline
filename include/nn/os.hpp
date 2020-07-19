/**
 * @file os.hpp
 * @brief Operating System implementations.
 */

#pragma once

#include "os.h"
#include "time.h"
#include "types.h"

namespace nn {
namespace os {
    namespace detail {
        class InternalCriticalSection {
            u32 Image;
        };

        class InternalConditionVariable {
            u32 Image;
        };
    }  // namespace detail

    typedef u64 Tick;
    typedef u64 LightEventType;

    // https://github.com/misson20000/nn-types/blob/master/nn_os.h
    struct EventType {
        nn::os::EventType* _x0;
        nn::os::EventType* _x8;
        bool isSignaled;
        bool initiallySignaled;
        bool shouldAutoClear;
        bool isInit;
        u32 signalCounter;
        u32 signalCounter2;
        nn::os::detail::InternalCriticalSection crit;
        nn::os::detail::InternalConditionVariable condvar;
    };
    typedef EventType Event;

    enum EventClearMode { EventClearMode_ManualClear, EventClearMode_AutoClear };

    struct ThreadType {
        u8 _0[0x40];
        u32 State;
        bool _44;
        bool _45;
        u8 _46;
        u32 PriorityBase;
        void* StackBase;
        void* Stack;
        size_t StackSize;
        void* Arg;
        u64 ThreadFunc;
        u8 _88[0x100];
        char Name[0x20];
        detail::InternalCriticalSection Crit;
        detail::InternalConditionVariable Condvar;
        u32 Handle;
        u8 padding[0x18];

        ThreadType(){};
    };
    static_assert(sizeof(ThreadType) == 0x1C0, "");

    struct MessageQueueType {
        u64 _x0;
        u64 _x8;
        u64 _x10;
        u64 _x18;
        void* Buffer;
        u32 MaxCount;
        u32 Count;
        u32 Offset;
        bool Initialized;
        detail::InternalCriticalSection _x38;
        detail::InternalConditionVariable _x3C;
        detail::InternalConditionVariable _x40;
    };

    struct ConditionVariableType {};

    struct SystemEvent;
    struct SystemEventType;

    // ARG
    void SetHostArgc(s32);
    s32 GetHostArgc();
    void SetHostArgv(char**);
    char** GetHostArgv();

    // MEMORY
    void InitializeVirtualAddressMemory();
    Result AllocateAddressRegion(u64*, u64);
    Result AllocateMemory(u64*, u64);
    Result AllocateMemoryPages(u64, u64);
    void AllocateMemoryBlock(u64*, u64);
    void FreeMemoryBlock(u64, u64);
    void SetMemoryHeapSize(u64);

    // MUTEX
    typedef struct MutexType {
        nnosMutexType impl;
    } MutexType;

    void InitializeMutex(nn::os::MutexType*, bool, s32);
    void FinalizeMutex(nn::os::MutexType*);
    void LockMutex(nn::os::MutexType*);
    bool TryLockMutex(nn::os::MutexType*);
    void UnlockMutex(nn::os::MutexType*);
    bool IsMutexLockedByCurrentThread(nn::os::MutexType const*);

    // QUEUE
    void InitializeMessageQueue(nn::os::MessageQueueType*, u64* buf, u64 queueCount);
    void FinalizeMessageQueue(nn::os::MessageQueueType*);

    bool TrySendMessageQueue(MessageQueueType*, u64);
    void SendMessageQueue(MessageQueueType*, u64);
    bool TimedSendMessageQueue(MessageQueueType*, u64, nn::TimeSpan);

    bool TryReceiveMessageQueue(u64* out, MessageQueueType*);
    void ReceiveMessageQueue(u64* out, MessageQueueType*);
    bool TimedReceiveMessageQueue(u64* out, MessageQueueType*, nn::TimeSpan);

    bool TryPeekMessageQueue(u64*, MessageQueueType const*);
    void PeekMessageQueue(u64*, MessageQueueType const*);
    bool TimedPeekMessageQueue(u64*, MessageQueueType const*);

    bool TryJamMessageQueue(nn::os::MessageQueueType*, u64);
    void JamMessageQueue(nn::os::MessageQueueType*, u64);
    bool TimedJamMessageQueue(nn::os::MessageQueueType*, u64, nn::TimeSpan);

    // CONDITION VARIABLE
    void InitializeConditionVariable(ConditionVariableType*);
    void FinalizeConditionVariable(ConditionVariableType*);

    void SignalConditionVariable(ConditionVariableType*);
    void BroadcastConditionVariable(ConditionVariableType*);
    void WaitConditionVariable(ConditionVariableType*);
    u8 TimedWaitConditionVariable(ConditionVariableType*, MutexType*, nn::TimeSpan);

    // THREAD
    Result CreateThread(nn::os::ThreadType*, void (*)(void*), void* arg, void* srcStack, u64 stackSize, s32 priority,
                        s32 coreNum);
    void DestroyThread(nn::os::ThreadType*);
    void StartThread(nn::os::ThreadType*);
    void SetThreadName(nn::os::ThreadType*, char const* threadName);
    void SetThreadNamePointer(nn::os::ThreadType*, char const*);
    char* GetThreadNamePointer(nn::os::ThreadType const*);
    nn::os::ThreadType* GetCurrentThread();
    s32 ChangeThreadPriority(nn::os::ThreadType* thread, s32 priority);
    s32 GetThreadPriority(nn::os::ThreadType const* thread);
    void YieldThread();
    void SuspendThread(nn::os::ThreadType*);
    void ResumeThread(nn::os::ThreadType*);
    void SleepThread(nn::TimeSpan);

    // EVENTS
    void InitializeEvent(EventType*, bool initiallySignaled, EventClearMode clearMode);
    void FinalizeEvent(EventType*);
    void SignalEvent(EventType*);
    void WaitEvent(EventType*);
    bool TryWaitEvent(EventType*);
    bool TimedWaitEvent(EventType*, nn::TimeSpan);
    void ClearEvent(EventType*);

    // EXCEPTION HANDLING
    typedef union {
        u64 x;  ///< 64-bit AArch64 register view.
        u32 w;  ///< 32-bit AArch64 register view.
        u32 r;  ///< AArch32 register view.
    } CpuRegister;
    /// Armv8 NEON register.

    typedef union {
        u128 v;    ///< 128-bit vector view.
        double d;  ///< 64-bit double-precision view.
        float s;   ///< 32-bit single-precision view.
    } FpuRegister;

    struct UserExceptionInfo {
        u32 ErrorDescription;  ///< See \ref ThreadExceptionDesc.
        u32 pad[3];

        CpuRegister CpuRegisters[29];  ///< GPRs 0..28. Note: also contains AArch32 registers.
        CpuRegister FP;                ///< Frame pointer.
        CpuRegister LR;                ///< Link register.
        CpuRegister SP;                ///< Stack pointer.
        CpuRegister PC;                ///< Program counter (elr_el1).

        u64 padding;

        FpuRegister FpuRegisters[32];  ///< 32 general-purpose NEON registers.

        u32 PState;  ///< pstate & 0xFF0FFE20
        u32 AFSR0;
        u32 AFSR1;
        u32 ESR;

        CpuRegister FAR;  ///< Fault Address Register.
    };
    void SetUserExceptionHandler(void (*)(UserExceptionInfo*), void*, ulong, UserExceptionInfo*);

    // OTHER
    void GenerateRandomBytes(void*, u64);
    nn::os::Tick GetSystemTick();
    u64 GetThreadAvailableCoreMask();
    void SetMemoryHeapSize(u64 size);

    namespace detail {
        extern s32 g_CommandLineParameter;
        extern char** g_CommandLineParameterArgv;
    };  // namespace detail
};      // namespace os
};      // namespace nn