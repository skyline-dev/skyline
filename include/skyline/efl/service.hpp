#pragma once

#include "eiffel/sl.h"
#include "skyline/nx/sf/service.h"

namespace skyline::efl {

class EflSlService {
   private:
    static constexpr Result SERVICE_INIT_FAILED = 0x42069114;

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
};

inline Result Log(const char* moduleName, EiffelLogLevel level, const char* content) {
    return EflSlService::GetInstance().Log(moduleName, level, content);
}

}  // namespace skyline::efl
