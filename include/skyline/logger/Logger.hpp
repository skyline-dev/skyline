#pragma once

#include <cstring>
#include <queue>
#include <string>

#include "nn/os.hpp"
#include "types.h"

namespace skyline::logger {
class Logger;
extern Logger* s_Instance;

class Logger {
   public:
    virtual void Initialize() = 0;
    virtual void SendRaw(void*, size_t) = 0;
    virtual std::string FriendlyName() = 0;

#ifndef NOLOG
    void StartThread();
    void Log(const char* data, size_t size = UINT32_MAX);
    void Log(std::string str);
    void LogFormat(const char* format, ...);
    void SendRaw(const char*);
    void SendRawFormat(const char*, ...);
    void Flush();
#else
    inline void StartThread() {}
    inline void Log(const char* data, size_t size = UINT32_MAX) {}
    inline void Log(std::string str) {}
    inline void LogFormat(const char* format, ...) {}
    inline void SendRaw(const char*) {}
    inline void SendRawFormat(const char*, ...) {}
    inline void Flush() {}
#endif
};
};  // namespace skyline::logger
