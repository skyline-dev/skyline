#pragma once

#include "types.h"
#include "CRC32.h"

#include "../utils/cpputils.hpp"

#include <unordered_map>
#include <string>
#include <string.h>

namespace skyline {
namespace arc {
    class Hashes {
        public:

        std::unordered_map<uint, std::string> loadedHashes;

        Hashes();

        bool hasCrc(uint);
        bool hasHash40(ulong);

        std::string lookupByCrc(uint);
        std::string lookupByHash40(ulong);

        ~Hashes();
    };
};
}