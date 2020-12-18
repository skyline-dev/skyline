#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define EIFFEL_SKYLINE_SERVICE_NAME "efl:sl"

typedef enum {
    EFL_SL_CMD_LOG = 0,
    EFL_SL_CMD_REGISTER_PLUGIN = 1,
    EFL_SL_CMD_REGISTER_SHARED_MEM = 2,
} EiffelSlCommandId;

typedef enum {
    EFL_LOG_LEVEL_INFO = 0,
    EFL_LOG_LEVEL_WARNING = 1,
    EFL_LOG_LEVEL_ERROR = 2,
} EiffelLogLevel;

typedef struct {
    SlPluginName name;
    uint64_t size;
    uint32_t perm;
} EiffelSlRegisterSharedMemIn;

#ifdef __cplusplus
}
#endif
