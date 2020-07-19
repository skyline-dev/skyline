#include "skyline/inlinehook/memcpy_controlled.hpp"

#include <string.h>

extern "C" Result sky_memcpy(void* dest, const void* src, size_t n) {
    if (dest == NULL || src == NULL) {
        return -1;
    }

    skyline::inlinehook::ControlledPages control(dest, n);

    control.claim();

    memcpy(control.rw, src, n);

    control.unclaim();

    return 0;
}
