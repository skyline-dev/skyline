#include "skyline/plugin/PluginManager.hpp"

namespace skyline {
namespace plugin {

    void Manager::Init() {
        Result rc;

        skyline::logger::s_Instance->LogFormat("[PluginManager] Initializing plugins...");
        std::unordered_map<std::string, PluginInfo> plugins;
        skyline::utils::walkDirectory(
            utils::g_RomMountStr + "skyline/plugins",
            [&plugins](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
                if (entry.type == nn::fs::DirectoryEntryType_File) plugins[*path] = PluginInfo();
            });

        skyline::logger::s_Instance->LogFormat("[PluginManager] Opening plugins...");
        for (auto& kv : plugins) {
            std::string path = kv.first;
            PluginInfo& plugin = kv.second;

            // grab file size
            nn::fs::FileHandle handle;
            rc = nn::fs::OpenFile(&handle, path.c_str(), nn::fs::OpenMode_Read);
            if (R_FAILED(rc)) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to open '%s' (0x%x). Skipping.",
                                                       path.c_str(), rc);
                continue;
            }

            s64 fileSize;
            nn::fs::GetFileSize(&fileSize, handle);
            nn::fs::CloseFile(handle);

            plugin.Size = fileSize;
            plugin.Data = memalign(0x1000, plugin.Size);

            skyline::utils::readFile(path, 0, plugin.Data, plugin.Size);
            skyline::logger::s_Instance->LogFormat("[PluginManager] Read %s", path.c_str());
        }

        size_t nrrSize = ALIGN_UP(sizeof(nn::ro::NrrHeader) + (plugins.size() * sizeof(utils::Sha256Hash)), 0x1000);

        nn::ro::NrrHeader nrr;
        memset(&nrr, 0, sizeof(nn::ro::NrrHeader));

        u64 program_id;
        svcGetInfo(&program_id, 18, INVALID_HANDLE, 0);

        nrr.magic = 0x3052524E;  // NRR0
        nrr.program_id = {program_id};
        nrr.type = 0;  // ForSelf
        nrr.hashes_offset = sizeof(nn::ro::NrrHeader);
        nrr.num_hashes = plugins.size();
        nrr.size = nrrSize;

        char* nrrBin = (char*)memalign(0x1000, nrrSize);

        skyline::logger::s_Instance->LogFormat("[PluginManager] Calculating hashes...");
        std::vector<utils::Sha256Hash> sortedHashes;
        for (auto& kv : plugins) {
            PluginInfo& plugin = kv.second;
            nn::ro::NroHeader* nro = (nn::ro::NroHeader*)plugin.Data;
            nn::crypto::GenerateSha256Hash(&plugin.Hash, sizeof(utils::Sha256Hash), nro, nro->size);
            sortedHashes.push_back(plugin.Hash);
        }
        std::sort(sortedHashes.begin(), sortedHashes.end());

        utils::Sha256Hash* hashes = reinterpret_cast<utils::Sha256Hash*>((u64)(nrrBin) + nrr.hashes_offset);
        for (auto& hash : sortedHashes) *hashes++ = hash;

        memcpy(nrrBin, &nrr, sizeof(nn::ro::NrrHeader));

        nn::ro::RegistrationInfo reg;
        rc = nn::ro::RegisterModuleInfo(&reg, nrrBin);
        if (R_FAILED(rc)) {
            free(nrrBin);
            skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to register NRR (0x%x).", rc);
            return;
        }

        skyline::logger::s_Instance->Log("[PluginManager] Loading plugins...\n");
        for (auto& kv : plugins) {
            PluginInfo& plugin = kv.second;

            size_t bufferSize;
            nn::ro::GetBufferSize(&bufferSize, plugin.Data);

            void* buffer = memalign(0x1000, bufferSize);

            rc = nn::ro::LoadModule(&plugin.Module, plugin.Data, buffer, bufferSize, nn::ro::BindFlag_Now);
            if (!rc) {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Loaded %s", kv.first.c_str(),
                                                       &plugin.Module.Name);
            } else {
                skyline::logger::s_Instance->LogFormat("[PluginManager] Failed to load %s, return code: 0x%x",
                                                       kv.first.c_str(), rc);
                // TODO: Don't attempt to run plugin main if we fail to load.
                // plugins.erase(kv.first);
            }
        }

        for (auto& kv : plugins) {
            PluginInfo& plugin = kv.second;

            skyline::logger::s_Instance->LogFormat("[PluginManager] Running `main` for %s", kv.first.c_str(),
                                                   &plugin.Module.Name);

            void (*pluginEntrypoint)() = NULL;
            rc = 0;
            rc = nn::ro::LookupModuleSymbol(reinterpret_cast<uintptr_t*>(&pluginEntrypoint), &plugin.Module, "main");
            if (pluginEntrypoint != NULL && !rc) {
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
