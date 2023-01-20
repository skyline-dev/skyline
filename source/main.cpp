#include "main.hpp"

#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/ipc.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/utils/utils.h"
#include "skyline/utils/call_once.hpp"
#include "skyline/utils/cur_proc_handle.hpp"

// For handling exceptions
char ALIGNA(0x1000) exception_handler_stack[0x4000];
nn::os::UserExceptionInfo exception_info;

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

static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);
    // TODO: Some games (Fire Emblem Three Houses, Pokemon Sword/Shield if I'm not mistaken, ...) use the &Config variant before arriving here and do not appreciate this. A shared lock between both impls would be convenient.
    nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

    skyline::logger::s_Instance->StartThread();

    // Result rc = nn::fs::MountSdCardForDebug("sd");
    // skyline::logger::s_Instance->LogFormat("[skyline_main] Mounted SD (0x%x)", rc);

    // load plugins
    // Note: Bypassing the singleton-like system because some older games (Final Fantasy 9) seem to have issues with _cxa_guard_acquire which gcc automatically adds when using the static instance
    auto manager = new skyline::plugin::Manager();
    manager->LoadPluginsImpl();
}};

void stub() {}

static skyline::utils::Once g_MountRomInit;
Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = 0;
    rc = nnFsMountRomImpl(path, buffer, size);

    skyline::utils::g_RomMountStr = std::string(path) + ":/";

    // Some games such as Persona 5 Royal call this method multiple times, so we have to ensure we only initialize the queue once
    g_MountRomInit.call_once([]() {
        // start task queue
        skyline::utils::SafeTaskQueue* taskQueue = new skyline::utils::SafeTaskQueue(100);
        taskQueue->startThread(20, 3, 0x10000); // Stack size 0x4000 -> 0x10000
        taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));
        nn::os::WaitEvent(&after_romfs_task->completionEvent);
    });

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
        skyline::logger::s_Instance->LogFormat("[skyline_main] Ran hooked nn::ro::initialize (0x%x)", ret);
    });

    return ret;
}

void skyline_main() {
    // populate our own process handle
    envSetOwnProcessHandle(skyline::proc_handle::Get());

    // init hooking setup
    A64HookInit();

    skyline::logger::setup_socket_hooks();

    // initialize logger
    nn::fs::MountSdCardForDebug("sd");
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\n");

    // override exception handler to dump info
    nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, sizeof(exception_handler_stack),
                                    &exception_info);

    // hook to prevent the game from double mounting romfs
    A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), reinterpret_cast<void*>(handleNnFsMountRom),
                    (void**)&nnFsMountRomImpl);

    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), reinterpret_cast<void*>(nn_ro_init), (void**)&nnRoInitializeImpl);

    // hook abort to get crash info
    // Note: This was commented out because some games do not use or have certain variations of this symbol.
    // This needs to be reworked to check for which symbol is available beforehand.
    // Until then, check Atmosphere's crash reports or Ryujinx's output
    /*
        uintptr_t VAbort_ptr = 0;
        nn::ro::LookupSymbol(&VAbort_ptr, "_ZN2nn4diag6detail10VAbortImplEPKcS3_S3_iPKNS_6ResultEPKNS_2os17UserExceptionInfoES3_RSt9__va_list");
        A64HookFunction(reinterpret_cast<void*>(VAbort_ptr), reinterpret_cast<void*>(handleNnDiagDetailVAbortImpl), (void**)&VAbortImpl);
    */

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
