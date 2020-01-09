/**
 * @file os.h
 * @brief Operating System implementations.
 */

#pragma once

#include "types.h"
#include "time.h"

namespace nn
{
    namespace os
    {
        namespace detail 
        {
            class InternalCriticalSection 
            {
                u32 Image;
            };

            class InternalConditionVariable 
            {
                u32 Image;
            };
        }

        typedef u64 Tick;
        typedef u64 LightEventType;

        // https://github.com/misson20000/nn-types/blob/master/nn_os.h
        struct EventType {
            nn::os::EventType *_x0;
            nn::os::EventType *_x8;
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
        
        struct ThreadType 
        {
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
            u8  _88[0x100];
            char Name[0x20];
            detail::InternalCriticalSection Crit;
            detail::InternalConditionVariable Condvar;
            u32 Handle;
            u8 padding[0x20];

            ThreadType() {};
        };

        struct MessageQueueType;
        struct SystemEvent;
        struct SystemEventType;

        // ARG
        void SetHostArgc(s32);
        s32 GetHostArgc();
        void SetHostArgv(char **);
        char** GetHostArgv();

        // MEMORY
        void InitializeVirtualAddressMemory();
        Result AllocateAddressRegion(u64 *, u64);
        Result AllocateMemory(u64 *, u64);
        Result AllocateMemoryPages(u64, u64);
        void AllocateMemoryBlock(u64 *, u64);
        void FreeMemoryBlock(u64, u64);
        void SetMemoryHeapSize(u64);

        // MUTEX
        struct MutexType
        {
            u8 curState; // _0
            bool isRecursiveMutex; // _1
            s32 lockLevel; // _2
            u8 _6[0x20-0xE];
        };

        void InitializeMutex(nn::os::MutexType *, bool, s32);
        void FinalizeMutex(nn::os::MutexType *);
        void LockMutex(nn::os::MutexType *);
        bool TryLockMutex(nn::os::MutexType *);
        void UnlockMutex(nn::os::MutexType *);
        bool IsMutexLockedByCurrentThread(nn::os::MutexType const *);

        // QUEUE
        void InitializeMessageQueue(nn::os::MessageQueueType *, u64 *buf, u64 queueCount);
        void FinalizeMessageQueue(nn::os::MessageQueueType *);
        bool TrySendMessageQueue(MessageQueueType* queue, u64* d);
        void SendMessageQueue(MessageQueueType* queue, u64* d);
        bool TryReceiveMessageQueue(u64* out, MessageQueueType* queue);
        void ReceiveMessageQueue(u64* out, MessageQueueType* queue);
        bool TryPeekMessageQueue(u64 *, nn::os::MessageQueueType const *);
        void PeekMessageQueue(u64 *, nn::os::MessageQueueType const *);
        bool TryJamMessageQueue(nn::os::MessageQueueType *, u64);
        void JamMessageQueue(nn::os::MessageQueueType *, u64);

        // THREAD
        Result CreateThread(nn::os::ThreadType *, void (*)(void *), void *arg, void *srcStack, u64 stackSize, s32 priority, s32 coreNum);
        void DestroyThread(nn::os::ThreadType *);
        void StartThread(nn::os::ThreadType *);
        void SetThreadName(nn::os::ThreadType *, char const *threadName);
        void SetThreadNamePointer(nn::os::ThreadType *, char const *);
        char* GetThreadNamePointer(nn::os::ThreadType const *);
        nn::os::ThreadType* GetCurrentThread();
        s32 ChangeThreadPriority(nn::os::ThreadType *thread, s32 priority);
        s32 GetThreadPriority(nn::os::ThreadType const *thread);
        void YieldThread();
        void SuspendThread(nn::os::ThreadType *);
        void ResumeThread(nn::os::ThreadType *);
        void SleepThread(nn::TimeSpan);

        // EXCEPTION HANDLING
        typedef union {
            u64 x; ///< 64-bit AArch64 register view.
            u32 w; ///< 32-bit AArch64 register view.
            u32 r; ///< AArch32 register view.
        } CpuRegister;
        /// Armv8 NEON register.

        typedef union {
            u128    v; ///< 128-bit vector view.
            double  d; ///< 64-bit double-precision view.
            float   s; ///< 32-bit single-precision view.
        } FpuRegister;

        struct UserExceptionInfo {
            u32 ErrorDescription;             ///< See \ref ThreadExceptionDesc.
            u32 pad[3];

            CpuRegister CpuRegisters[29];   ///< GPRs 0..28. Note: also contains AArch32 registers.
            CpuRegister FP;             ///< Frame pointer.
            CpuRegister LR;             ///< Link register.
            CpuRegister SP;             ///< Stack pointer.
            CpuRegister PC;             ///< Program counter (elr_el1).

            u64 padding;

            FpuRegister FpuRegisters[32];   ///< 32 general-purpose NEON registers.

            u32 PState;                 ///< pstate & 0xFF0FFE20
            u32 AFSR0;
            u32 AFSR1;
            u32 ESR;

            CpuRegister FAR;            ///< Fault Address Register.
        };
        void SetUserExceptionHandler(void(*)(UserExceptionInfo*), void*, ulong, UserExceptionInfo*);

        // OTHER
        void GenerateRandomBytes(void *, u64);
        nn::os::Tick GetSystemTick();
        u64 GetThreadAvailableCoreMask();
        void SetMemoryHeapSize(u64 size);

        namespace detail
        {
            extern s32 g_CommandLineParameter;
            extern char** g_CommandLineParameterArgv;
        };
    };
};