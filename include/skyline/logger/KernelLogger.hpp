#pragma once

#include <string>

#include "skyline/logger/Logger.hpp"

namespace skyline::logger {
class KernelLogger : public Logger {
   public:
    KernelLogger();

    virtual void Initialize();
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "KernelLogger"; }
};
};  // namespace skyline::logger