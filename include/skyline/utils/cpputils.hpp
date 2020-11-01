#pragma once

#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "nn/fs.h"
#include "nn/settings.h"
#include "operator.h"
#include "types.h"

namespace skyline::utils {

enum region : u8 { Text, Rodata, Data, Bss, Heap };

extern std::string g_RomMountStr;

extern u64 g_MainTextAddr;
extern u64 g_MainRodataAddr;
extern u64 g_MainDataAddr;
extern u64 g_MainBssAddr;
extern u64 g_MainHeapAddr;

extern nn::settings::system::FirmwareVersion g_CachedFwVer;

void init();

Result walkDirectory(std::string const&,
                     std::function<void(nn::fs::DirectoryEntry const&, std::shared_ptr<std::string>)>,
                     bool recursive = true);
Result readEntireFile(std::string const&, void**, size_t*);
Result readFile(std::string const&, s64, void*, size_t);
Result writeFile(std::string const&, s64, void*, size_t);
Result entryCount(u64*, std::string const&, nn::fs::DirectoryEntryType);
extern "C" void* getRegionAddress(skyline::utils::region);

struct Sha256Hash {
    u8 hash[0x20];

    bool operator==(const Sha256Hash& o) const { return std::memcmp(this, &o, sizeof(*this)) == 0; }
    bool operator!=(const Sha256Hash& o) const { return std::memcmp(this, &o, sizeof(*this)) != 0; }
    bool operator<(const Sha256Hash& o) const { return std::memcmp(this, &o, sizeof(*this)) < 0; }
    bool operator>(const Sha256Hash& o) const { return std::memcmp(this, &o, sizeof(*this)) > 0; }
};
};  // namespace skyline::utils