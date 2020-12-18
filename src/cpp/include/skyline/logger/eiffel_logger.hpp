#pragma once

#include <string>

#include "Logger.hpp"

namespace skyline::logger {

class EiffelLogger : public Logger {
   public:
    EiffelLogger();

    virtual void Initialize();
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "EiffelLogger"; }
};

};  // namespace skyline::logger
