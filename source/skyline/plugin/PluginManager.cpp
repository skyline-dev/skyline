#include "skyline/plugin/PluginManager.hpp"

namespace skyline {
namespace Plugin {

    void Manager::Init() {
        skyline::TcpLogger::LogFormat("Initializing plugins...");
        std::unordered_map<std::string, PluginInfo> plugins;
        skyline::Utils::walkDirectory("rom:/skyline/plugins", [&plugins](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
            if(entry.type == nn::fs::DirectoryEntryType_File)
                plugins[*path] = PluginInfo();
        });
        
        skyline::TcpLogger::LogFormat("Opening plugins...");
        for(auto& kv : plugins){
            std::string path = kv.first;
            PluginInfo& plugin = kv.second;

            // grab file size
            nn::fs::FileHandle handle;
            nn::fs::OpenFile(&handle, path.c_str(), nn::fs::OpenMode_Read);
            s64 fileSize;
            nn::fs::GetFileSize(&fileSize, handle);
            nn::fs::CloseFile(handle);

            plugin.Size = fileSize;
            plugin.Data = memalign(0x1000, plugin.Size);
            skyline::Utils::readFile(path, 0, plugin.Data, plugin.Size);
            skyline::TcpLogger::LogFormat("Read %s", path.c_str());
        }

        size_t nrrSize = ALIGN_UP(sizeof(nn::ro::NrrHeader) + (plugins.size() * sizeof(Utils::Sha256Hash)), 0x1000);

        nn::ro::NrrHeader nrr;
        memset(&nrr, 0, sizeof(nn::ro::NrrHeader));

        nrr.magic = 0x3052524E; // NRR0
        nrr.program_id = {0x01006A800016E000ul};
        nrr.type = 0; // ForSelf
        nrr.hashes_offset = sizeof(nn::ro::NrrHeader);
        nrr.num_hashes = plugins.size();
        nrr.size = nrrSize;

        char* nrrBin = (char*) memalign(0x1000, nrrSize);

        skyline::TcpLogger::Log("Calculating hashes...");
        std::vector<Utils::Sha256Hash> sortedHashes;
        for(auto& kv : plugins){
            PluginInfo& plugin = kv.second;
            nn::ro::NroHeader* nro = (nn::ro::NroHeader*) plugin.Data;
            nn::crypto::GenerateSha256Hash(&plugin.Hash, sizeof(Utils::Sha256Hash), nro, nro->size);
            sortedHashes.push_back(plugin.Hash);
        }
        std::sort(sortedHashes.begin(), sortedHashes.end());
        
        Utils::Sha256Hash* hashes = reinterpret_cast<Utils::Sha256Hash*>((u64)(nrrBin) + nrr.hashes_offset);
        for(auto& hash : sortedHashes)
            *hashes++ = hash;

        memcpy(nrrBin, &nrr, sizeof(nn::ro::NrrHeader));
        
        //skyline::Utils::writeFile("sd:/test.nrr", 0, (void*) nrrBin, nrrSize);

        nn::ro::RegistrationInfo reg;
        Result r = nn::ro::RegisterModuleInfo(&reg, nrrBin);
        skyline::TcpLogger::Log("Registered the NRR.");

        skyline::TcpLogger::Log("Loading plugins...");
        for(auto &kv : plugins){
            PluginInfo& plugin = kv.second;

            size_t bufferSize;
            nn::ro::GetBufferSize(&bufferSize, plugin.Data);

            void* buffer = memalign(0x1000, bufferSize);

            r = nn::ro::LoadModule(&plugin.Module, plugin.Data, buffer, bufferSize, nn::ro::BindFlag_Now);
            skyline::TcpLogger::LogFormat("Loaded %s", kv.first.c_str());
        }
    }

};
};