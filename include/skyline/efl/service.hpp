#pragma once

#include "eiffel/sl.h"
#include "skyline/nx/sf/service.h"

namespace skyline::efl {

class EflSlService {
   private:
    static constexpr Result SERVICE_INIT_FAILED = MAKERESULT(Module_Skyline, SkylineError_SlServiceInitFailed);
    static constexpr Result INVALID_PLUGIN_NAME = MAKERESULT(Module_Skyline, SkylineError_InvalidPluginName);

    EflSlService();
    EflSlService(const EflSlService&) = delete;
    ~EflSlService();

    Service m_service;
    bool m_initSuccessful;

   public:
    inline static auto& GetInstance() {
        static EflSlService s_instance;
        return s_instance;
    }

    Result Log(const char* moduleName, EiffelLogLevel level, const char* logContent);
    Result RegisterPlugin(SlPluginMeta meta);
    Result RegisterSharedMem(const SlPluginName name, SlPluginSharedMemInfo sharedMemInfo);
};

inline Result Log(const char* moduleName, EiffelLogLevel level, const char* content) {
    return EflSlService::GetInstance().Log(moduleName, level, content);
}

inline Result RegisterPlugin(SlPluginMeta meta) { return EflSlService::GetInstance().RegisterPlugin(meta); }

inline Result RegisterSharedMem(const SlPluginName name, SlPluginSharedMemInfo sharedMemInfo) {
    return EflSlService::GetInstance().RegisterSharedMem(name, sharedMemInfo);
}

}  // namespace skyline::efl
