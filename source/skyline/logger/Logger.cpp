#include "skyline/logger/Logger.hpp"

#include <cstdarg>

#include "alloc.h"
#include "mem.h"
#include "operator.h"

namespace skyline::logger {

Logger* s_Instance;

#ifndef NOLOG

std::queue<char*>* g_msgQueue = nullptr;

void ThreadMain(void* arg) {
    Logger* t = (Logger*)arg;

    t->Initialize();

    t->LogFormat("[%s] Logger initialized.", t->FriendlyName().c_str());

    while (true) {
        t->Flush();
        nn::os::YieldThread();  // let other parts of OS do their thing
        nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(100000000));
    }
}

void Logger::StartThread() {
    const size_t stackSize = 0x3000;
    void* threadStack = memalign(0x1000, stackSize);

    nn::os::ThreadType* thread = new nn::os::ThreadType;
    nn::os::CreateThread(thread, ThreadMain, this, threadStack, stackSize, 16, 0);
    nn::os::StartThread(thread);
}

void Logger::SendRaw(const char* data) { SendRaw((void*)data, strlen(data)); }

void Logger::SendRawFormat(const char* format, ...) {
    va_list args;
    char buff[0x1000] = {0};
    va_start(args, format);

    int len = vsnprintf(buff, sizeof(buff), format, args);

    SendRaw(buff, len);

    va_end(args);
}

void AddToQueue(char* data) {
    if (!g_msgQueue) g_msgQueue = new std::queue<char*>();

    g_msgQueue->push(data);
}

void Logger::Flush() {
    if (!g_msgQueue) return;

    while (!g_msgQueue->empty()) {
        auto data = g_msgQueue->front();

        SendRaw(data, strlen(data));
        delete[] data;
        g_msgQueue->pop();
    }
}

void Logger::Log(const char* data, size_t size) {
    if (size == UINT32_MAX) size = strlen(data);

    char* ptr = new char[size + 2];
    memset(ptr, 0, size + 2);
    memcpy(ptr, data, size);
    // ptr[size] = '\n';

    AddToQueue(ptr);
    return;
}

void Logger::Log(std::string str) { Log(str.data(), str.size()); }

void Logger::LogFormat(const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t len = vsnprintf(NULL, 0, format, args);
    char* ptr = new char[len + 2];
    memset(ptr, 0, len + 2);
    vsnprintf(ptr, len + 1, format, args);
    ptr[len] = '\n';

    AddToQueue(ptr);
    va_end(args);

    return;
}

#endif  // NOLOG

};  // namespace skyline::logger
