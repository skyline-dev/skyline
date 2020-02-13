#include "skyline/arc/Hashes.hpp"

#include "skyline/utils/cpputils.hpp"
#include "nn/fs.h"

namespace skyline {
namespace arc {
    std::unordered_map<u32, std::string> loadedHashes;

    Hashes::Hashes(){ 
               
        skyline::utils::walkDirectory("rom:/", [this](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
            std::string crcpath = path->substr(5); // remove "rom:/"

            loadedHashes[createFromPath(crcpath)] = crcpath;
        });

        constexpr const char* hashesPath = "rom:/skyline/hashes.txt";
        char* hashesBin;
        Result r = skyline::utils::readEntireFile(hashesPath, reinterpret_cast<void**>(&hashesBin), NULL);
        if(R_SUCCEEDED(r)){
        
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
            skyline::TcpLogger::Log("[HashesLoader] Hashes loaded.");
        } else 
            skyline::TcpLogger::LogFormat("[HashesLoader] Failed to open \'%s\' (0x%x).", hashesPath, r);
    }

    bool Hashes::hasCrc(u32 hash){
        return loadedHashes.find(hash) != loadedHashes.end();
    }

    bool Hashes::hasHash40(skyline::arc::Arc::Hash40 hash40){
        return hasCrc(hash40.Hash);
    }

    std::string Hashes::lookupByCrc(u32 hash){
        
        if(hasCrc(hash))
            return loadedHashes[hash];
        else {
            std::stringstream stream;
            stream << "0x" << std::hex << hash;
            return stream.str();
        }

    }

    std::string Hashes::lookupByHash40(skyline::arc::Arc::Hash40 hash){
        std::string s = lookupByCrc(hash.Hash);
        if(s.find("0x", 0) == 0){
            std::stringstream stream;
            stream << "0x" << std::hex << (u64) hash;
            return stream.str();
        } else
            return s;
    }
    skyline::arc::Arc::Hash40 Hashes::create(std::string path){
        skyline::arc::Arc::Hash40 hash;

        hash.Length = path.size();
        hash.Hash = crc32(path.c_str(), hash.Length);

        return hash;
    }

    skyline::arc::Arc::Hash40 Hashes::createFromPath(std::string path){
        
        
        // detect "mounts"
        /*
        std::set<std::string> mounts = {"stream", "prebuilt"};
        for(auto& prefix : mounts){
            if(crcpath.rfind(prefix, 0) != std::string::npos){ // does it start mount prefix?

                crcpath.insert(prefix.size(), 1, ':'); // insert ':' after mount
                break;
            }
        }*/

        return create(path);
    }
};
};