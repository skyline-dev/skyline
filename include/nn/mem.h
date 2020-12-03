/**
 * @file mem.h
 * @brief Memory allocation functions.
 */

#pragma once

#include "os.hpp"
#include "types.h"

namespace nn {
namespace mem {
    class StandardAllocator {
       public:
        StandardAllocator();

        void Initialize(void* address, u64 size);
        void Finalize();
        void* Reallocate(void* address, u64 newSize);
        void* Allocate(u64 size);
        void* Allocate(u64 size, u64 alignment);
        void Free(void* address);
        void Dump();

        bool mIsInitialized;         // _0
        bool mIsEnabledThreadCache;  // _1
        u16 _2;
        u64* mAllocAddr;  // _4
        u8 _12[20];
    };
};  // namespace mem
};  // namespace nn
