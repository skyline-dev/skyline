#pragma once

#include "types.h"
#include "CRC32.h"
#include "Arc.hpp"

#include "skyline/logger/TcpLogger.hpp"

#include <unordered_map>
#include <string>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <set>


namespace skyline {
namespace arc {
    class Hashes {
        public:

        std::unordered_map<u32, std::string> loadedHashes;

        Hashes();

        bool hasCrc(u32);
        bool hasHash40(skyline::arc::Arc::Hash40);

        std::string lookupByCrc(u32);
        std::string lookupByHash40(skyline::arc::Arc::Hash40);

        static skyline::arc::Arc::Hash40 create(std::string);
        static skyline::arc::Arc::Hash40 createFromPath(std::string );

        ~Hashes();
    };
};
}