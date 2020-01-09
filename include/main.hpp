#pragma once

#include "types.h"
#include "ModuleObject.hpp"
#include "mem.h"
#include "alloc.h"
#include "operator.h"

#include "skyline/nx/kernel/svc.h"
#include "skyline/nx/kernel/virtmem.h"
#include "skyline/nx/runtime/env.h"

#include <map>
#include <string>

#include "nn/nn.h"
#include "nn/os.h"
#include "nn/oe.h"
#include "nn/fs.h"
#include "nn/ro.h"
#include "nn/crypto.h"

#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/TcpLogger.hpp"
#include "skyline/arc/Hashes.hpp"
#include "skyline/plugin/PluginManager.hpp"

void skylineMain();