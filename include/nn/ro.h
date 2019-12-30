/**
 * @file ro.h
 * @brief Dynamic module API.
 */

#pragma once

#include "types.h"

namespace nn 
{
    class ro 
    {
        public:
        
        static void LookupSymbol(ulong *, char const*);
    };

};