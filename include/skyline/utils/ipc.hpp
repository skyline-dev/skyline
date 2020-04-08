#pragma once

#include "types.h"

namespace skyline::utils {
    class Ipc {
        public:

        static Result getOwnProcessHandle(Handle*);
    };
}