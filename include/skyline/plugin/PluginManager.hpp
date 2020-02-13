#pragma once

#include "types.h"
#include "operator.h"
#include "alloc.h"
#include "mem.h"

#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/cpputils.hpp"

#include "nn/ro.h"
#include "nn/crypto.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>

namespace skyline {
namespace plugin {
    class PluginInfo {
        public:
        void*  Data;
        size_t Size;
        utils::Sha256Hash Hash;
        nn::ro::Module Module;
    };

    class Manager {
        public:
        static void Init();
    };
};
};