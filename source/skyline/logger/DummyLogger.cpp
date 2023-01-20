#include "skyline/logger/DummyLogger.hpp"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/kernel/svc.h"

#ifdef __cplusplus
}
#endif

namespace skyline::logger {

DummyLogger::DummyLogger() {}

void DummyLogger::Initialize() {
    // nothing to do
}

void DummyLogger::SendRaw(void* data, size_t size) {
};

bool DummyLogger::ShouldFlush() {
    return true;
}

};  // namespace skyline::logger