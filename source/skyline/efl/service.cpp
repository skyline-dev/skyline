#include "skyline/efl/service.hpp"

#include "skyline/logger/Logger.hpp"
#include "skyline/utils/ipc.hpp"

namespace skyline::efl {

EflSlService::EflSlService() : m_initSuccessful(false) {
    logger::s_Instance->Log("[EflSlService] Initializing...\n");
    auto rc = utils::nnServiceCreate(&m_service, EIFFEL_SKYLINE_SERVICE_NAME);
    if (R_SUCCEEDED(rc)) {
        m_initSuccessful = true;
        logger::s_Instance->LogFormat("[EflSlService] Initialized");
    } else {
        logger::s_Instance->LogFormat("[EflSlService] Initialization failed (0x%x)", rc);
    }
}

EflSlService::~EflSlService() {
    if (m_initSuccessful) {
        utils::nnServiceClose(&m_service);
    }
}

Result EflSlService::Log(const char* moduleName, EiffelLogLevel level, const char* logContent) {
    if (!m_initSuccessful) {
        return SERVICE_INIT_FAILED;
    }

    return nnServiceDispatchIn(&m_service, EFL_SL_CMD_LOG, level,
                               .buffer_attrs =
                                   {
                                       SfBufferAttr_HipcMapAlias | SfBufferAttr_In,
                                       SfBufferAttr_HipcMapAlias | SfBufferAttr_In,
                                   },
                               .buffers = {
                                   {moduleName, strlen(moduleName) + 1},
                                   {logContent, strlen(logContent) + 1},
                               });
}

Result EflSlService::RegisterPlugin(SlPluginMeta meta) {
    if (!m_initSuccessful) {
        return SERVICE_INIT_FAILED;
    }

    if (!(strnlen(meta.name, SL_PLUGIN_NAME_SIZE) < SL_PLUGIN_NAME_SIZE)) {
        return INVALID_PLUGIN_NAME;
    }

    return nnServiceDispatchIn(&m_service, EFL_SL_CMD_REGISTER_PLUGIN, meta);
}

Result EflSlService::RegisterSharedMem(const SlPluginName name, SlPluginSharedMemInfo sharedMemInfo) {
    if (!m_initSuccessful) {
        return SERVICE_INIT_FAILED;
    }

    if (!(strnlen(name, SL_PLUGIN_NAME_SIZE) < SL_PLUGIN_NAME_SIZE)) {
        return INVALID_PLUGIN_NAME;
    }

    auto in = EiffelSlRegisterSharedMemIn{{}, sharedMemInfo.size, sharedMemInfo.perm};
    strcpy(in.name, name);
    return nnServiceDispatchIn(&m_service, EFL_SL_CMD_REGISTER_SHARED_MEM, in, .in_num_handles = 1,
                               .in_handles = {sharedMemInfo.handle});
}

}  // namespace skyline::efl
