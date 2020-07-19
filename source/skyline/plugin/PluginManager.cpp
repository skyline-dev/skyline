#include "skyline/plugin/PluginManager.hpp"

#include <set>

#include "nn/crypto.h"
#include "skyline/logger/TcpLogger.hpp"

namespace skyline::plugin {

    void Manager::LoadPluginsImpl() {
        Result rc;

        skyline::logger::s_Instance->LogFormat("[PluginManager] Initializing plugins...");
        
        // walk through romfs:/skyline/plugins recursively to find any files and push them into map
        skyline::utils::walkDirectory(utils::g_RomMountStr + "skyline/plugins",
                                      [this](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
                                          if (entry.type == nn::fs::DirectoryEntryType_File)  // ignore directories
                                              m_pluginInfos.push_back(PluginInfo{.Path = *path});
                                      });
        
        skyline::logger::s_Instance->LogFormat("[PluginManager] Opening plugins...");
        auto pluginInfoIter = m_pluginInfos.begin();
        while (pluginInfoIter != m_pluginInfos.end()) {
            auto& plugin = *pluginInfoIter;

            // open file
            nn::fs::FileHandle handle;
            rc = nn::fs::OpenFile(&handle, plugin.Path.c_str(), nn::fs::OpenMode_Read);
            
            // file couldn't be opened, bail
            if (R_FAILED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to open '%s' (0x%x). Skipping.",
                                                       plugin.Path.c_str(), rc);
                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            s64 fileSize;
            rc = nn::fs::GetFileSize(&fileSize, handle);
            nn::fs::CloseFile(handle); // file should be closed regardless of anything failing
            
            // getting file size failed, bail
            if (R_FAILED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to get '%s' size. (0x%x). Skipping.",
                                                       plugin.Path.c_str(), rc);

                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            plugin.Size = fileSize;
            plugin.Data = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.Size));

            rc = skyline::utils::readFile(plugin.Path, 0, plugin.Data.get(), plugin.Size);
            if (R_SUCCEEDED(rc))
                skyline::logger::s_Instance->LogFormat("[PluginManager] Read %s", plugin.Path.c_str());
            else {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to read '%s'. (0x%x). Skipping.",
                                                       plugin.Path.c_str(), rc);
                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }
            
            pluginInfoIter++;
        }

        skyline::logger::s_Instance->LogFormat("[PluginManager] Calculating hashes...");

        // ro requires hashes to be sorted
        std::set<utils::Sha256Hash> sortedHashes;
        pluginInfoIter = m_pluginInfos.begin();
        while (pluginInfoIter != m_pluginInfos.end()) {
            auto& plugin = *pluginInfoIter;
            nn::ro::NroHeader* nroHeader = (nn::ro::NroHeader*)plugin.Data.get();
            nn::crypto::GenerateSha256Hash(&plugin.Hash, sizeof(utils::Sha256Hash), nroHeader, nroHeader->size);

            if (sortedHashes.find(plugin.Hash) != sortedHashes.end()) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] %s is detected duplicate, ignoring...",
                                                       plugin.Path.c_str());

                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            sortedHashes.insert(plugin.Hash);
            pluginInfoIter++;
        }

        // (sizeof(nrr header) + sizeof(sha256) * plugin count) aligned by 0x1000, as required by ro
        m_nrrSize = ALIGN_UP(sizeof(nn::ro::NrrHeader) + (m_pluginInfos.size() * sizeof(utils::Sha256Hash)), 0x1000);

        // get our own program ID
        // TODO: dedicated util for this
        u64 program_id;
        svcGetInfo(&program_id, 18, INVALID_HANDLE, 0);
        
        nn::ro::NrrHeader nrr = {
            .magic = 0x3052524E, // NRR0
            .program_id = {program_id},
            .size = (u32)m_nrrSize,
            .type = 0, // ForSelf
            .hashes_offset = sizeof(nn::ro::NrrHeader),
            .num_hashes = (u32)m_pluginInfos.size(),
        };
        
        m_nrrBuffer = std::unique_ptr<u8>((u8*)memalign(0x1000, m_nrrSize)); // must be page aligned 
        memset(m_nrrBuffer.get(), 0, m_nrrSize);

        // copy hashes into nrr
        utils::Sha256Hash* hashes = reinterpret_cast<utils::Sha256Hash*>((u64)(m_nrrBuffer.get()) + nrr.hashes_offset);
        auto curHashIdx = 0;
        for (auto hash : sortedHashes) {
            hashes[curHashIdx++] = hash;
        }

        // copy header into nrr
        memcpy(m_nrrBuffer.get(), &nrr, sizeof(nn::ro::NrrHeader));
        
        // try to register plugins
        nn::ro::RegistrationInfo reg;
        rc = nn::ro::RegisterModuleInfo(&reg, m_nrrBuffer.get());
        
        if(R_FAILED(rc)){
            // ro rejected, free and bail
            m_nrrBuffer = nullptr;
            
            // free all loaded plugins
            m_pluginInfos.clear();
            
            skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to register NRR (0x%x).", rc);
            return;
        }

        skyline::logger::s_Instance->Log("[PluginManager] Loading plugins...\n");
        pluginInfoIter = m_pluginInfos.begin();
        while (pluginInfoIter != m_pluginInfos.end()) {
            auto& plugin = *pluginInfoIter;

            // attempt to get the required size for the bss 
            rc = nn::ro::GetBufferSize(&plugin.BssSize, plugin.Data.get());
            if (R_FAILED(rc)) {
                // ro rejected file, bail
                // (the original input is not validated to be an actual NRO, so this isn't unusual)
                skyline::logger::s_Instance->LogFormat(
                    "[PluginManager] nn::ro::GetBufferSize failed on %s (0x%x), not an nro?", plugin.Path.c_str(), rc);
                
                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                
                continue;
            }

            plugin.BssData = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.BssSize)); // must be page aligned

            rc = nn::ro::LoadModule(
                &plugin.Module, plugin.Data.get(), plugin.BssData.get(), plugin.BssSize,
                nn::ro::BindFlag_Now);  // bind immediately, so all symbols are immediately available
 
            if (R_SUCCEEDED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Loaded %s", plugin.Path.c_str(),
                                                       &plugin.Module.Name);
            } else {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to load %s, return code: 0x%x",
                                                       plugin.Path.c_str(), rc);

                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            pluginInfoIter++;
        }

        pluginInfoIter = m_pluginInfos.begin();
        while (pluginInfoIter != m_pluginInfos.end()) {
            auto& plugin = *pluginInfoIter;

            skyline::logger::s_Instance->LogFormat("[PluginManager] Running `main` for %s", plugin.Path.c_str(),
                                                   &plugin.Module.Name);

            // try to find entrypoint
            void (*pluginEntrypoint)() = NULL;
            rc = nn::ro::LookupModuleSymbol(
                reinterpret_cast<uintptr_t*>(&pluginEntrypoint),
                &plugin.Module,
                "main");
            
            if (pluginEntrypoint != NULL && R_SUCCEEDED(rc)) {
                pluginEntrypoint();
                skyline::logger::s_Instance->LogFormat("[PluginManager] Finished running `main` for %s, rc: 0x%x",
                                                       plugin.Path.c_str(), rc);
            } else {
                skyline::logger::s_Instance->LogFormat(
                    "[PluginManager] Failed to lookup symbol for %s, return code: 0x%x", plugin.Path.c_str(), rc);
            }
            
            pluginInfoIter++;
        }
    }

};
