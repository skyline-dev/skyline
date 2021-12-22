#pragma once

#include <string>

#include <arpa/inet.h>

#include "alloc.h"
#include "mem.h"
#include "nn/socket.h"
#include "nn/time.h"
#include "operator.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/Logger.hpp"

namespace skyline::logger {
class DualLogger : public Logger {
   public:
    DualLogger(std::string);

    virtual void Initialize();
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "DualLogger"; }
};
};  // namespace skyline::logger