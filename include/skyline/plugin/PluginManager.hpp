#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>

#include "alloc.h"
#include "mem.h"
#include "nn/crypto.h"
#include "nn/ro.h"
#include "operator.h"
#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/cpputils.hpp"
#include "types.h"

namespace skyline::plugin {

class PluginInfo {
   public:
    void* Data;
    size_t Size;
    utils::Sha256Hash Hash;
    nn::ro::Module Module;
};

class Manager {
   public:
    static void Init();
};
};  // namespace skyline::plugin