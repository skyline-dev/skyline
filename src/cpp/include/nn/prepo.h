#pragma once

#include "account.h"
#include "types.h"

namespace nn {
namespace prepo {
    class PlayReport {
       public:
        char m_EventName[0x20];
        void* m_Buff;
        size_t m_BuffLength;
        u64 m_End;

        PlayReport();
        Result SetEventId(char const*);
        Result SetBuffer();

        Result Add(char const*, long);
        Result Add(char const*, double);
        Result Add(char const*, char const*);

        Result Save();
        Result Save(account::Uid const&);
    };
};  // namespace prepo
};  // namespace nn