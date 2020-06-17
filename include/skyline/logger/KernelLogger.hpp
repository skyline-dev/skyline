#pragma once

#include "skyline/logger/Logger.hpp"

#include <string>

namespace skyline::logger {
    class KernelLogger : public Logger
    {
        public:

        KernelLogger();

        virtual void Initialize();
        virtual void SendRaw(void*, size_t);
        virtual std::string FriendlyName() {
            return "KernelLogger";
        }
    };
};