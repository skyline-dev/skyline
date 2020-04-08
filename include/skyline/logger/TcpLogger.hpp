
#pragma once

#include "operator.h"
#include "alloc.h"
#include "mem.h"

#include "nn/socket.h"
#include "nn/time.h"

#include "skyline/inlinehook/And64InlineHook.hpp"

#include "skyline/logger/Logger.hpp"

#include <arpa/inet.h>

namespace skyline::logger {
    class TcpLogger : public Logger
    {
        public:
        virtual void Initialize();
        virtual void SendRaw(void*, size_t);
        virtual std::string FriendlyName() {
            return "TcpLogger";
        }
    };
};