#pragma once

#include "types.h"
#include "utils.h"
#include "operator.h"

#include "nn/fs.h"

#include <cstring>
#include <functional>
#include <string>
#include <memory>

namespace skyline {
    class Utils {
        public:
        static Result walkDirectory(std::string const&, std::function<void(nn::fs::DirectoryEntry const&,  std::shared_ptr<std::string>)>);
        static Result readEntireFile(std::string const&, void**, size_t*);
        static Result readFile(std::string const&, s64, void*, size_t);
        static Result writeFile(std::string const&, s64, void*, size_t);
        
        struct Sha256Hash {
            u8 hash[0x20];

            bool operator==(const Sha256Hash &o) const {
                return std::memcmp(this, &o, sizeof(*this)) == 0;
            }
            bool operator!=(const Sha256Hash &o) const {
                return std::memcmp(this, &o, sizeof(*this)) != 0;
            }
            bool operator<(const Sha256Hash &o) const {
                return std::memcmp(this, &o, sizeof(*this)) < 0;
            }
            bool operator>(const Sha256Hash &o) const {
                return std::memcmp(this, &o, sizeof(*this)) > 0;
            }
        };
    };
};