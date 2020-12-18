#include "skyline/logger/eiffel_logger.hpp"

#include <string.h>

#include "skyline/efl/service.hpp"

namespace skyline::logger {

EiffelLogger::EiffelLogger() {}

void EiffelLogger::Initialize() {}

void EiffelLogger::SendRaw(void* data, size_t size) {
    skyline::efl::Log("skyline::logger", EFL_LOG_LEVEL_INFO, static_cast<const char*>(data));
};

};  // namespace skyline::logger
