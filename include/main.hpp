#pragma once

#include "mem.h"
#include "alloc.h"
#include "operator.h"
#include "types.h"
#include "skyline/nx/kernel/svc.h"
#include "skyline/nx/kernel/virtmem.h"
#include "skyline/nx/runtime/env.h"
#include "skyline/nx/kernel/condvar.h"
#include "nn/os.h"
#include "nn/oe.h"
#include "nn/fs.h"
#include "nn/ro.h"
#include "nn/prepo.h"
#include "nn/nn.h"
#include "ModuleObject.hpp"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/TcpLogger.hpp"

void skylineMain();