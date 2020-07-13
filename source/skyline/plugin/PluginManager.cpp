#include "skyline/plugin/PluginManager.hpp"

namespace skyline {
namespace plugin {

    void Manager::Init() {
        Result rc;

        skyline::logger::s_Instance->LogFormat("[PluginManager] Initializing plugins...");
        
        std::unordered_map<std::string, PluginInfo> plugins;
        
        // walk through romfs:/skyline/plugins recursively to find any files and push them into map
        skyline::utils::walkDirectory(utils::g_RomMountStr + "skyline/plugins", 
        [&plugins](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
            if(entry.type == nn::fs::DirectoryEntryType_File) // ignore directories
                plugins[*path] = PluginInfo();
        });
        
        skyline::logger::s_Instance->LogFormat("[PluginManager] Opening plugins...");
        for (auto& kv : plugins) {
            std::string path = kv.first;
            PluginInfo& plugin = kv.second;

            // open file
            nn::fs::FileHandle handle;
            rc = nn::fs::OpenFile(&handle, path.c_str(), nn::fs::OpenMode_Read);
            
            // file couldn't be opened, bail
            if(R_FAILED(rc)){
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to open '%s' (0x%x). Skipping.", path.c_str(), rc);
                
                // stop tracking
                plugins.erase(kv.first);
                continue;
            }

            s64 fileSize;
            rc = nn::fs::GetFileSize(&fileSize, handle);
            nn::fs::CloseFile(handle); // file should be closed regardless of anything failing
            
            // getting file size failed, bail
            if(R_FAILED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to get '%s' size. (0x%x). Skipping.", path.c_str(), rc);
                
                // stop tracking
                plugins.erase(kv.first);
                
                continue;
            }

            plugin.Size = fileSize;
            plugin.Data = memalign(0x1000, plugin.Size);

            rc = skyline::utils::readFile(path, 0, plugin.Data, plugin.Size);
            if(R_SUCCEEDED(rc))
                skyline::logger::s_Instance->LogFormat("[PluginManager] Read %s", path.c_str());
            else {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to read '%s'. (0x%x). Skipping.", path.c_str(), rc);
                
                // free space allocated for the file
                free(plugin.Data);
                
                // stop tracking
                plugins.erase(kv.first);
            }
        }

        // (sizeof(nrr header) + sizeof(sha256) * plugin count) aligned by 0x1000, as required by ro
        size_t nrrSize = ALIGN_UP(sizeof(nn::ro::NrrHeader) + (plugins.size() * sizeof(utils::Sha256Hash)), 0x1000);

        // get our own program ID
        // TODO: dedicated util for this
        u64 program_id;
        svcGetInfo(&program_id, 18, INVALID_HANDLE, 0);
        
        nn::ro::NrrHeader nrr = {
            .magic = 0x3052524E, // NRR0
            .program_id = {program_id},
            .size = nrrSize,
            .type = 0, // ForSelf
            .hashes_offset = sizeof(nn::ro::NrrHeader),
            .num_hashes = plugins.size(),
        };
        
        char* nrrBin = (char*) memalign(0x1000, nrrSize); // must be page aligned 

        skyline::logger::s_Instance->LogFormat("[PluginManager] Calculating hashes...");
        std::vector<utils::Sha256Hash> sortedHashes;
        for (auto& kv : plugins) {
            PluginInfo& plugin = kv.second;
            nn::ro::NroHeader* nro = (nn::ro::NroHeader*)plugin.Data;
            nn::crypto::GenerateSha256Hash(&plugin.Hash, sizeof(utils::Sha256Hash), nro, nro->size);
            sortedHashes.push_back(plugin.Hash);
        }
        // sort hashes, as required by ro
        std::sort(sortedHashes.begin(), sortedHashes.end());
        
        // copy hashes into nrr
        utils::Sha256Hash* hashes = reinterpret_cast<utils::Sha256Hash*>((u64)(nrrBin) + nrr.hashes_offset);
        memcpy(hashes, sortedHashes.data(), sizeof(utils::Sha256Hash) * sortedHashes.size());

        // copy header into nrr
        memcpy(nrrBin, &nrr, sizeof(nn::ro::NrrHeader));
        
        // try to register plugins
        nn::ro::RegistrationInfo reg;
        rc = nn::ro::RegisterModuleInfo(&reg, nrrBin);
        
        if(R_FAILED(rc)){
            // ro rejected, free and bail
            free(nrrBin);
            
            // free all loaded plugins
            for(auto& kv : plugins)
                free(kv.second.Data);
            
            skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to register NRR (0x%x).", rc);
            return;
        }

        skyline::logger::s_Instance->Log("[PluginManager] Loading plugins...\n");
        for (auto& kv : plugins) {
            PluginInfo& plugin = kv.second;

            // attempt to get the required size for the bss 
            size_t bssSize;
            if(R_FAILED(nn::ro::GetBufferSize(&bssSize, plugin.Data))) {
                // ro rejected file, bail (the original input is not validated to be an actual NRO, so this isn't unusual)
                
                // we don't need the file data any more
                free(plugin.Data);
                
                // stop tracking
                plugins.erase(kv.first);
                
                continue;
            }

            void* bss = memalign(0x1000, bssSize); // must be page aligned 

            rc = nn::ro::LoadModule(&plugin.Module, plugin.Data, bss, bufferSize, nn::ro::BindFlag_Now);// bind immediately, so all symbols are immediately available
 
            if(R_SUCCEEDED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Loaded %s", kv.first.c_str(), &plugin.Module.Name);
            } else {
               skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to load %s, return code: 0x%x", kv.first.c_str(), rc);
                
               // couldn't be loaded, free unused memory
               free(bss);
               free(plugin.Data);
                
                // stop tracking
               plugins.erase(kv.first);
            }
        }

        for (auto& kv : plugins) {
            PluginInfo& plugin = kv.second;

            skyline::logger::s_Instance->LogFormat("[PluginManager] Running `main` for %s", kv.first.c_str(),
                                                   &plugin.Module.Name);

            // try to find entrypoint
            void (*pluginEntrypoint)() = NULL;
            rc = nn::ro::LookupModuleSymbol(
                reinterpret_cast<uintptr_t*>(&pluginEntrypoint),
                &plugin.Module,
                "main");
            
            if(pluginEntrypoint != NULL && R_SUCCEEDED(rc)) {
                pluginEntrypoint();
                skyline::logger::s_Instance->LogFormat("[PluginManager] Finished running `main` for %s, rc: 0x%x",
                                                       kv.first.c_str(), rc);
            } else {
                skyline::logger::s_Instance->LogFormat(
                    "[PluginManager] Failed to lookup symbol for %s, return code: 0x%x", kv.first.c_str(), rc);
            }
        }
    }
};  // namespace plugin
};  // namespace skyline
