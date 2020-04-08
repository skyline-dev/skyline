#pragma once

#include "types.h"
#include "nn/os.hpp"

#include <string>
#include <queue>
#include <cstring>

namespace skyline::logger {
    class Logger;
    extern Logger* s_Instance;

    class Logger {
        public:

        virtual void Initialize() = 0;
        virtual void SendRaw(void*, size_t) = 0;
        virtual std::string FriendlyName() = 0;
        
        void StartThread();
        void Log(const char* data, size_t size = UINT32_MAX);
        void Log(std::string str);
        void LogFormat(const char* format, ...);
        void SendRaw(const char*);
        void SendRawFormat(const char*, ...);
        void Flush();
    };
};