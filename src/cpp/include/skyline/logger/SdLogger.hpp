
#pragma once

#include <string>

#include "skyline/logger/Logger.hpp"

namespace skyline::logger {
class SdLogger : public Logger {
   public:
    SdLogger(std::string);

    virtual void Initialize();
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "SdLogger"; }
};
};  // namespace skyline::logger