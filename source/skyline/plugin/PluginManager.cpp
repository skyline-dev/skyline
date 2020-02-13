#include "skyline/plugin/PluginManager.hpp"

namespace skyline {
namespace plugin {

    void Manager::Init() {
        Result rc;

        skyline::TcpLogger::LogFormat("[PluginManager] Initializing plugins...");
        std::unordered_map<std::string, PluginInfo> plugins;
        skyline::utils::walkDirectory(utils::g_RomMountStr + "skyline/plugins", [&plugins](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
            if(entry.type == nn::fs::DirectoryEntryType_File)
                plugins[*path] = PluginInfo();
        });
        
        skyline::TcpLogger::LogFormat("[PluginManager] Opening plugins...");
        for(auto& kv : plugins){
            std::string path = kv.first;
            PluginInfo& plugin = kv.second;

            // grab file size
            nn::fs::FileHandle handle;
            rc = nn::fs::OpenFile(&handle, path.c_str(), nn::fs::OpenMode_Read);
            if(R_FAILED(rc)){
                skyline::TcpLogger::LogFormat("[PluginManager] Failed to open '%s' (0x%x). Skipping.", path.c_str(), rc);
                continue;
            }

            s64 fileSize;
            nn::fs::GetFileSize(&fileSize, handle);
            nn::fs::CloseFile(handle);

            plugin.Size = fileSize;
            plugin.Data = memalign(0x1000, plugin.Size);

            skyline::utils::readFile(path, 0, plugin.Data, plugin.Size);
            skyline::TcpLogger::LogFormat("[PluginManager] Read %s", path.c_str());
        }

        size_t nrrSize = ALIGN_UP(sizeof(nn::ro::NrrHeader) + (plugins.size() * sizeof(utils::Sha256Hash)), 0x1000);

        nn::ro::NrrHeader nrr;
        memset(&nrr, 0, sizeof(nn::ro::NrrHeader));

        nrr.magic = 0x3052524E; // NRR0
        nrr.program_id = {0x01006A800016E000ul};
        nrr.type = 0; // ForSelf
        nrr.hashes_offset = sizeof(nn::ro::NrrHeader);
        nrr.num_hashes = plugins.size();
        nrr.size = nrrSize;

        char* nrrBin = (char*) memalign(0x1000, nrrSize);

        skyline::TcpLogger::Log("[PluginManager] Calculating hashes...");
        std::vector<utils::Sha256Hash> sortedHashes;
        for(auto& kv : plugins){
            PluginInfo& plugin = kv.second;
            nn::ro::NroHeader* nro = (nn::ro::NroHeader*) plugin.Data;
            nn::crypto::GenerateSha256Hash(&plugin.Hash, sizeof(utils::Sha256Hash), nro, nro->size);
            sortedHashes.push_back(plugin.Hash);
        }
        std::sort(sortedHashes.begin(), sortedHashes.end());
        
        utils::Sha256Hash* hashes = reinterpret_cast<utils::Sha256Hash*>((u64)(nrrBin) + nrr.hashes_offset);
        for(auto& hash : sortedHashes)
            *hashes++ = hash;

        memcpy(nrrBin, &nrr, sizeof(nn::ro::NrrHeader));
        
        nn::ro::RegistrationInfo reg;
        rc = nn::ro::RegisterModuleInfo(&reg, nrrBin);
        if(R_FAILED(rc)){
            free(nrrBin);
            skyline::TcpLogger::Log("[PluginManager] Failed to register NRR (0x%x).", rc);
            return;
        }

        skyline::TcpLogger::Log("[PluginManager] Loading plugins...");
        for(auto &kv : plugins){
            PluginInfo& plugin = kv.second;

            size_t bufferSize;
            nn::ro::GetBufferSize(&bufferSize, plugin.Data);

            void* buffer = memalign(0x1000, bufferSize);

            rc = nn::ro::LoadModule(&plugin.Module, plugin.Data, buffer, bufferSize, nn::ro::BindFlag_Now);
            skyline::TcpLogger::LogFormat("[PluginManager] Loaded %s", kv.first.c_str());

            void (*pluginEntrypoint)();
            nn::ro::LookupModuleSymbol(
                reinterpret_cast<uintptr_t*>(&pluginEntrypoint), 
                &plugin.Module, 
                "");
        }
    }

};
};