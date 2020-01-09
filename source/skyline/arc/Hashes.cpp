#include "skyline/arc/Hashes.hpp"

#include "nn/fs.h"

namespace skyline {
namespace arc {
    std::unordered_map<u32, std::string> loadedHashes;

    Hashes::Hashes(){
        skyline::Utils::walkDirectory("rom:/", [this](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
            std::string const& crcpath = path->substr(5); // remove "rom:/"

            u32 crc = crc32(crcpath.c_str(), crcpath.length());
            loadedHashes[crc] = crcpath;
        });

        char* hashesBin;
        R_ERRORONFAIL(skyline::Utils::readEntireFile("rom:/skyline/hashes.txt", reinterpret_cast<void**>(&hashesBin), NULL));
        
        std::string hashesStr(hashesBin);
        
        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = hashesStr.find('\n', prev)) != std::string::npos)
        {
            std::string path = hashesStr.substr(prev, pos - prev);
            u32 crc = crc32(path.c_str(), path.length());
            loadedHashes[crc] = path;
            
            prev = pos + 1;
        }

        free(hashesBin);
    }

    bool Hashes::hasCrc(uint hash){
        return loadedHashes.find(hash) != loadedHashes.end();
    }

    std::string Hashes::lookupByCrc(uint hash){
        return loadedHashes[hash];
    }

    std::string Hashes::lookupByHash40(ulong hash){
        uint crc = (u32) hash & 0xFFFFFFFF;
        return lookupByCrc(crc);
    }
};
};