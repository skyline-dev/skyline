#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../nx/kernel/svc.h"
#include "../nx/arm/tls.h"

#ifdef __cplusplus
}
#endif

namespace skyline::proc_handle {
    Handle Get();
}