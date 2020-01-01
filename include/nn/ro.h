/**
 * @file ro.h
 * @brief Dynamic module API.
 */

#pragma once

#include "types.h"
#include "ModuleObject.hpp"

namespace nn 
{
    namespace ro 
    { 
        class Module {
            public:
            rtld::ModuleObject* ModuleObject;
            u32 State;
            void* NroPtr;
            void* BssPtr;
            void *_x20;
            void *SourceBuffer;
            char Name[256]; /* Created by retype action */
            u8 _x130;
            u8 _x131;
            bool isLoaded; // bool
        };

        void LookupSymbol(ulong *, char const*);

        Result LoadModule(Module*, void const*, void*, ulong, int);
        Result UnloadModule(Module*);
    };

};