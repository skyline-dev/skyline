#pragma once

#include "types.h"

namespace nn 
{
    namespace prepo 
    {
        class PlayReport {
            public:
            
            PlayReport();
            void SetEventId();
            void SetBuffer();

            void Add(char const*, long);
            void Add(char const*, double);
            void Add(char const*, char const*);
        };
    };
};