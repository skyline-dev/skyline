#pragma once

#include "types.h"
#include "ModuleObject.hpp"
#include "mem.h"
#include "alloc.h"
#include "operator.h"

#include <map>
#include <string>

#include "nn/nn.h"
#include "nn/os.hpp"
#include "nn/oe.h"
#include "nn/fs.h"
#include "nn/ro.h"
#include "nn/crypto.h"
#include "nn/prepo.h"
#include "nvn/pfnc.h"

#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/TcpLogger.hpp"
#include "skyline/plugin/PluginManager.hpp"
#include "skyline/utils/SafeQueue.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/runtime/env.h"
#include "skyline/nx/kernel/virtmem.h"

#ifdef __cplusplus
};
#endif

extern nn::os::EventType romMountedEvent;

extern "C" void skyline_init();