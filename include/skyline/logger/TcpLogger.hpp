
#pragma once

#include <arpa/inet.h>
#include <queue>
#include <string>
#include <cstring>
#include <cstdarg>

#include "alloc.h"
#include "mem.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "nn/socket.h"
#include "nn/os.h"
#include "nn/time.h"

#include "operator.h"

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
        static void ClearQueue();
    };
};