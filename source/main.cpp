#include "main.hpp"

#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/ipc.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/utils/utils.h"
#include "skyline/utils/call_once.hpp"
#include "skyline/utils/SymbolMap.hpp"

// For handling exceptions
char ALIGNA(0x1000) exception_handler_stack[0x4000];
nn::os::UserExceptionInfo exception_info;

const char* RomMountName = "rom";

void exception_handler(nn::os::UserExceptionInfo* info) {
    skyline::logger::s_Instance->LogFormat("Exception occurred!\n");

    skyline::logger::s_Instance->LogFormat("Error description: %x\n", info->ErrorDescription);
    for (int i = 0; i < 29; i++)
        skyline::logger::s_Instance->LogFormat("X[%02i]: %" PRIx64 "\n", i, info->CpuRegisters[i].x);
    skyline::logger::s_Instance->LogFormat("FP: %" PRIx64 "\n", info->FP.x);
    skyline::logger::s_Instance->LogFormat("LR: %" PRIx64 "\n", info->LR.x);
    skyline::logger::s_Instance->LogFormat("SP: %" PRIx64 "\n", info->SP.x);
    skyline::logger::s_Instance->LogFormat("PC: %" PRIx64 "\n", info->PC.x);
}

void* (*lookupGlobalManualImpl)(const char* symName);

void* handleLookupGlobalManual(const char* symName) {
    void* result = lookupGlobalManualImpl(symName);
    if (result == nullptr) {
        uintptr_t mapValue = skyline::utils::SymbolMap::getSymbolAddress(std::string(symName));
        return reinterpret_cast<void*>(mapValue);
    }
    return result;
}

Result (*handleLookupSymbolImpl)(uintptr_t* pOutAddress, const char* name);

Result handleLookupSymbol(uintptr_t* pOutAddress, const char* name) {
    Result res = handleLookupSymbolImpl(pOutAddress, name);
    if (R_FAILED(res)) {
        uintptr_t mapValue = skyline::utils::SymbolMap::getSymbolAddress(std::string(name));
        if (mapValue != 0) {
            *pOutAddress = mapValue;
            return 0;
        }
    }

    return res;
}

static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);
    nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

    skyline::logger::s_Instance->StartThread();

    Result rc = nn::fs::MountSdCardForDebug("sd");
    skyline::logger::s_Instance->LogFormat("[skyline_main] Mounted SD (0x%x)", rc);

    // Load symbol map
    if (skyline::utils::SymbolMap::tryLoad()) {
        // If a symbol map was loaded, hook the global symbol lookup function
        // Apparently, this function isn't called for every symbol, but always if a symbol couldn't be found
        uintptr_t lookupGlobalManualPtr;
        nn::ro::LookupSymbol(&lookupGlobalManualPtr, "_ZN2nn2ro6detail18LookupGlobalManualEPKc");
        if (lookupGlobalManualPtr != 0) {
            A64HookFunction(reinterpret_cast<void*>(lookupGlobalManualPtr),
                reinterpret_cast<void*>(handleLookupGlobalManual), reinterpret_cast<void**>(&lookupGlobalManualImpl));
        } else {
            skyline::logger::s_Instance->LogFormat("[skyline_main] Failed to hook nn::ro::detail::LookupGlobalManual. "
                "Symbols from maps cannot be used.");
        }

        // Also handle manual calls to nn::ro::LookupSymbol
        A64HookFunction(reinterpret_cast<void*>(nn::ro::LookupSymbol), reinterpret_cast<void*>(handleLookupSymbol),
           reinterpret_cast<void**>(&handleLookupSymbolImpl));

        skyline::logger::s_Instance->LogFormat("[skyline_main] Installed symbol map hooks.");
    }

    // load plugins
    skyline::plugin::Manager::LoadPlugins();
    skyline::logger::s_Instance->LogFormat("[skyline_main] loaded plugins");
}};

void stub() {}

Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = 0;
    rc = nnFsMountRomImpl(path, buffer, size);

    skyline::utils::g_RomMountStr = std::string(path) + ":/";

    // start task queue
    skyline::utils::SafeTaskQueue* taskQueue = new skyline::utils::SafeTaskQueue(100);
    taskQueue->startThread(20, 3, 0x4000);
    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));
    nn::os::WaitEvent(&after_romfs_task->completionEvent);
    return rc;
}

void (*VAbortImpl)(char const*, char const*, char const*, int, Result const*, nn::os::UserExceptionInfo const*, char const*, va_list args);
void handleNnDiagDetailVAbortImpl(char const* str1, char const* str2, char const* str3, int int1, Result const* code, nn::os::UserExceptionInfo const* ExceptionInfo, char const* fmt, va_list args) {
    int len = vsnprintf(nullptr, 0, fmt, args);
    char* fmt_info = new char[len + 1];
    vsprintf(fmt_info, fmt, args);

    const char* fmt_str = "%s\n%s\n%s\n%d\nError: 0x%x\n%s";
    len = snprintf(nullptr, 0, fmt_str, str1, str2, str3, int1, *code, fmt_info);
    char* report = new char[len + 1];
    sprintf(report, fmt_str, str1, str2, str3, int1, *code, fmt_info);

    skyline::logger::s_Instance->LogFormat("%s", report);
    nn::err::ApplicationErrorArg* error =
        new nn::err::ApplicationErrorArg(69, "The software is aborting.", report,
                                         nn::settings::LanguageCode::Make(nn::settings::Language::Language_English));
    nn::err::ShowApplicationError(*error);
    delete[] report;
    delete[] fmt_info;
    VAbortImpl(str1, str2, str3, int1, code, ExceptionInfo, fmt, args);
}

static skyline::utils::Once g_RoInit;
Result (*nnRoInitializeImpl)();

Result nn_ro_init() {
    Result ret = 0;

    g_RoInit.call_once([&ret]() {
         ret = nnRoInitializeImpl();
    });

    return ret;
}

void skyline_main() {
    // populate our own process handle
    Handle h;
    skyline::utils::Ipc::getOwnProcessHandle(&h);
    envSetOwnProcessHandle(h);

    // init hooking setup
    A64HookInit();

    skyline::logger::setup_socket_hooks();

    // initialize logger
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\n");

    // override exception handler to dump info
    nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, sizeof(exception_handler_stack),
                                    &exception_info);

    // hook to prevent the game from double mounting romfs
    A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), reinterpret_cast<void*>(handleNnFsMountRom),
                    (void**)&nnFsMountRomImpl);

    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), reinterpret_cast<void*>(nn_ro_init), (void**)&nnRoInitializeImpl);

    skyline::logger::s_Instance->LogFormat("[skyline_main] text: 0x%" PRIx64 " | rodata: 0x%" PRIx64
                                           " | data: 0x%" PRIx64 " | bss: 0x%" PRIx64 " | heap: 0x%" PRIx64,
                                           skyline::utils::g_MainTextAddr, skyline::utils::g_MainRodataAddr,
                                           skyline::utils::g_MainDataAddr, skyline::utils::g_MainBssAddr,
                                           skyline::utils::g_MainHeapAddr);

}

extern "C" void skyline_init() {
    skyline::utils::init();
    virtmemSetup();  // needed for libnx JIT

    skyline_main();
}
