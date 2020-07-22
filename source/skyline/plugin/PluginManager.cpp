#include "skyline/plugin/PluginManager.hpp"

#include <set>

#include "nn/crypto.h"
#include "skyline/logger/TcpLogger.hpp"

namespace skyline {
namespace plugin {

    void Manager::LoadPluginsImpl() {
        Result rc;

        skyline::logger::s_Instance->LogFormat("[PluginManager] Initializing plugins...");

        // walk through romfs:/skyline/plugins recursively to find any files and push them into map
        skyline::utils::walkDirectory(utils::g_RomMountStr + PLUGIN_PATH,
                                      [this](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
                                          if (entry.type == nn::fs::DirectoryEntryType_File)  // ignore directories
                                              m_pluginInfos.push_back(PluginInfo{.Path = *path});
                                      });

        if (m_pluginInfos.empty()) {
            skyline::logger::s_Instance->LogFormat("[PluginManager] No plugin to load.");
            return;
        }

        // open plugins
        skyline::logger::s_Instance->LogFormat("[PluginManager] Opening plugins...");

        std::set<utils::Sha256Hash> sortedHashes;  // ro requires hashes to be sorted

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
            nn::fs::CloseFile(handle);  // file should be closed regardless of anything failing

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

            // get the required size for the bss
            rc = nn::ro::GetBufferSize(&plugin.BssSize, plugin.Data.get());
            if (R_FAILED(rc)) {
                // ro rejected file, bail
                // (the original input is not validated to be an actual NRO, so this isn't unusual)
                skyline::logger::s_Instance->LogFormat(
                    "[PluginManager] Failed to get NRO buffer size for '%s' (0x%x), not an nro? Skipping.",
                    plugin.Path.c_str(), rc);

                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            // calculate plugin hashes
            nn::ro::NroHeader* nroHeader = (nn::ro::NroHeader*)plugin.Data.get();
            nn::crypto::GenerateSha256Hash(&plugin.Hash, sizeof(utils::Sha256Hash), nroHeader, nroHeader->size);

            if (sortedHashes.find(plugin.Hash) != sortedHashes.end()) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] '%s' is detected duplicate, Skipping.",
                                                       plugin.Path.c_str());
                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            sortedHashes.insert(plugin.Hash);

            pluginInfoIter++;
        }

        // build nrr and register plugins

        // (sizeof(nrr header) + sizeof(sha256) * plugin count) aligned by 0x1000, as required by ro
        m_nrrSize = ALIGN_UP(sizeof(nn::ro::NrrHeader) + (m_pluginInfos.size() * sizeof(utils::Sha256Hash)), 0x1000);
        m_nrrBuffer = std::unique_ptr<u8>((u8*)memalign(0x1000, m_nrrSize));  // must be page aligned
        memset(m_nrrBuffer.get(), 0, m_nrrSize);

        // get our own program ID
        // TODO: dedicated util for this
        u64 program_id;
        svcGetInfo(&program_id, 18, INVALID_HANDLE, 0);

        // initialize nrr header
        auto nrrHeader = reinterpret_cast<nn::ro::NrrHeader*>(m_nrrBuffer.get());
        *nrrHeader = nn::ro::NrrHeader{
            .magic = 0x3052524E,  // NRR0
            .program_id = {program_id},
            .size = (u32)m_nrrSize,
            .type = 0,  // ForSelf
            .hashes_offset = sizeof(nn::ro::NrrHeader),
            .num_hashes = (u32)m_pluginInfos.size(),
        };

        // copy hashes into nrr
        utils::Sha256Hash* hashes =
            reinterpret_cast<utils::Sha256Hash*>((size_t)m_nrrBuffer.get() + nrrHeader->hashes_offset);
        auto curHashIdx = 0;
        for (auto hash : sortedHashes) {
            hashes[curHashIdx++] = hash;
        }

        // register plugins
        rc = nn::ro::RegisterModuleInfo(&m_registrationInfo, m_nrrBuffer.get());

        if (R_FAILED(rc)) {
            // ro rejected, free and bail
            m_nrrBuffer = nullptr;

            // free all loaded plugins
            m_pluginInfos.clear();

            skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to register NRR (0x%x).", rc);
            return;
        }

        // load plugins

        skyline::logger::s_Instance->Log("[PluginManager] Loading plugins...\n");
        pluginInfoIter = m_pluginInfos.begin();
        while (pluginInfoIter != m_pluginInfos.end()) {
            auto& plugin = *pluginInfoIter;

            plugin.BssData = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.BssSize));  // must be page aligned

            rc = nn::ro::LoadModule(
                &plugin.Module, plugin.Data.get(), plugin.BssData.get(), plugin.BssSize,
                nn::ro::BindFlag_Now);  // bind immediately, so all symbols are immediately available

            if (R_SUCCEEDED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Loaded '%s'", plugin.Path.c_str(),
                                                       &plugin.Module.Name);
            } else {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to load '%s' (0x%x). Skipping.",
                                                       plugin.Path.c_str(), rc);
                // stop tracking
                pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
                continue;
            }

            pluginInfoIter++;
        }

        // execute plugin entrypoints

        pluginInfoIter = m_pluginInfos.begin();
        while (pluginInfoIter != m_pluginInfos.end()) {
            auto& plugin = *pluginInfoIter;

            skyline::logger::s_Instance->LogFormat("[PluginManager] Running `main` for %s", plugin.Path.c_str(),
                                                   &plugin.Module.Name);

            // try to find entrypoint
            void (*pluginEntrypoint)() = NULL;
            rc = nn::ro::LookupModuleSymbol(reinterpret_cast<uintptr_t*>(&pluginEntrypoint), &plugin.Module, "main");

            if (pluginEntrypoint != NULL && R_SUCCEEDED(rc)) {
                pluginEntrypoint();
                skyline::logger::s_Instance->LogFormat("[PluginManager] Finished running `main` for '%s' (0x%x)",
                                                       plugin.Path.c_str(), rc);
            } else {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to lookup symbol for '%s' (0x%x)",
                                                       plugin.Path.c_str(), rc);
            }

            pluginInfoIter++;
        }
    }

};  // namespace plugin
};  // namespace skyline
