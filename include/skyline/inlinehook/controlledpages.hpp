#pragma once

#include "types.h"

namespace skyline::inlinehook {
class ControlledPages {
   private:
    bool isClaimed;
    size_t size;

   public:
    void* rx;
    void* rw;

    ControlledPages(void* addr, size_t size);

    void claim();
    void unclaim();
};
};  // namespace skyline::inlinehook