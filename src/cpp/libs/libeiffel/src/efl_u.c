#include <string.h>

#include "eiffel/user.h"
#include "service_guard.h"

static Service g_eiffelSrv;

NX_GENERATE_SERVICE_GUARD(eiffel);

Result _eiffelInitialize() { return smGetService(&g_eiffelSrv, EIFFEL_USER_SERVICE_NAME); }

void _eiffelCleanup(void) { serviceClose(&g_eiffelSrv); }

Result eiffelGetPluginMeta(SlPluginMeta* out_pluginMeta, const SlPluginName name) {
    if (!(strnlen(name, SL_PLUGIN_NAME_SIZE) < SL_PLUGIN_NAME_SIZE)) return EFL_U_RESULT_BAD_PLUGIN_NAME;
    struct {
        SlPluginName name;
    } in;
    strcpy(in.name, name);
    return serviceDispatchInOut(&g_eiffelSrv, EFL_U_CMD_GET_PLUGIN_META, in, *out_pluginMeta);
}

Result eiffelGetPluginSharedMemInfo(SlPluginSharedMemInfo* out_pluginSharedMemInfo, const SlPluginName name) {
    if (!(strnlen(name, SL_PLUGIN_NAME_SIZE) < SL_PLUGIN_NAME_SIZE)) return EFL_U_RESULT_BAD_PLUGIN_NAME;
    struct {
        SlPluginName name;
    } in;
    strcpy(in.name, name);

    Handle out_handle = INVALID_HANDLE;
    Result rc = serviceDispatchInOut(&g_eiffelSrv, EFL_U_CMD_GET_PLUGIN_SHARED_MEM, in, *out_pluginSharedMemInfo,
                                     .out_handle_attrs = {SfOutHandleAttr_HipcMove}, .out_handles = &out_handle);
    if (R_SUCCEEDED(rc)) {
        out_pluginSharedMemInfo->handle = out_handle;
    }
    return rc;
}
