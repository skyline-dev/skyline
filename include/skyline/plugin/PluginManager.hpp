#pragma once

#include <list>
#include <memory>
#include <string>

#include "nn/ro.h"
#include "skyline/utils/cpputils.hpp"

namespace skyline::plugin {

    struct PluginInfo {
        std::string Path;
        void* Data;
        size_t Size;
        utils::Sha256Hash Hash;
        nn::ro::Module Module;
    };

    class Manager {
        private:
        std::list<PluginInfo> m_pluginInfos;

        static inline auto& GetInstance() {
            static Manager s_instance;
            return s_instance;
        }

        void LoadPluginsImpl();

        public:
        static inline void LoadPlugins() { GetInstance().LoadPluginsImpl(); }
    };

};
