#pragma once

#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define EIFFEL_USER_SERVICE_NAME "efl:u"
#define EIFFEL_MODULE_ID 0x144

typedef enum {
    EFL_U_CMD_GET_PLUGIN_META = 0,
    EFL_U_CMD_GET_PLUGIN_SHARED_MEM = 1,
} EiffelUserCommandId;

static const Result EFL_U_RESULT_BAD_PLUGIN_NAME = MAKERESULT(EIFFEL_MODULE_ID, 0);
static const Result EFL_U_RESULT_PLUGIN_NOT_ACTIVE = MAKERESULT(EIFFEL_MODULE_ID, 1);
static const Result EFL_U_RESULT_SHARED_MEM_NOT_REGISTERED = MAKERESULT(EIFFEL_MODULE_ID, 2);

Result eiffelInitialize();
void eiffelExit();

Result eiffelGetPluginMeta(SlPluginMeta* out_pluginMeta, const SlPluginName name);
Result eiffelGetPluginSharedMemInfo(SlPluginSharedMemInfo* out_pluginSharedMemInfo, const SlPluginName name);

#ifdef __cplusplus
}
#endif
