#pragma once

#include <string>

#include "skyline/logger/Logger.hpp"

namespace skyline::logger {
class DummyLogger : public Logger {
   public:
    DummyLogger();

    virtual void Initialize();
    virtual bool ShouldFlush() override;
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "DummyLogger"; }
};
};  // namespace skyline::logger