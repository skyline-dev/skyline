#pragma once

#include <list>
#include <memory>
#include <string>

#include "nn/ro.h"
#include "skyline/utils/cpputils.hpp"

namespace skyline {
namespace plugin {

    static constexpr auto PLUGIN_PATH = "skyline/plugins";

    struct PluginInfo {
        std::string Path;
        std::unique_ptr<u8> Data;
        size_t Size;
        utils::Sha256Hash Hash;
        nn::ro::Module Module;
        std::unique_ptr<u8> BssData;
        size_t BssSize;
    };

    class Manager {
       private:
        std::list<PluginInfo> m_pluginInfos;
        std::unique_ptr<u8> m_nrrBuffer;
        size_t m_nrrSize;
        nn::ro::RegistrationInfo m_registrationInfo;

        static inline auto& GetInstance() {
            static Manager s_instance;
            return s_instance;
        }

        void LoadPluginsImpl();

       public:
        static inline void LoadPlugins() { GetInstance().LoadPluginsImpl(); }
    };

};  // namespace plugin
};  // namespace skyline
