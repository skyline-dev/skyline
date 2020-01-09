
#pragma once

#include "operator.h"
#include "alloc.h"
#include "mem.h"

#include "nn/socket.h"
#include "nn/os.h"
#include "nn/time.h"

#include "skyline/inlinehook/And64InlineHook.hpp"

#include <queue>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <cstdarg>

namespace skyline {
    class TcpLogger
    {
        private:
        static void ThreadMain(void*);

        public:
        static void StartThread();

        static void Initialize();
        static void Log(const char* data, size_t size = UINT32_MAX);
        static void Log(std::string str);
        static void LogFormat(const char* format, ...);

        static void SendRaw(const char*);
        static void SendRawFormat(const char*, ...);
        static void SendRaw(void*, size_t);
        
        static void ClearQueue();
    };
};