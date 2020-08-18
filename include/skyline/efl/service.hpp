#pragma once

#include "eiffel/sl.h"
#include "skyline/nx/sf/service.h"

namespace skyline::efl {

class EflSlService {
   private:
    static constexpr auto SKYLINE_MODULE_NUMBER = 0x1A4;
    static constexpr Result SERVICE_INIT_FAILED = MAKERESULT(SKYLINE_MODULE_NUMBER, 0);
    static constexpr Result INVALID_PLUGIN_NAME = MAKERESULT(SKYLINE_MODULE_NUMBER, 1);

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
