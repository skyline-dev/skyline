#include "main.hpp"

#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/ipc.hpp"

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

static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    // mount sd
    Result rc = nn::fs::MountSdCardForDebug("sd");
    skyline::logger::s_Instance->LogFormat("[skyline_main] Mounted SD (0x%x)", rc);

    // load plugins
    skyline::plugin::Manager::LoadPlugins();
}};

void stub() {}

Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = 0;
    if(strcmp(path, RomMountName) == 0) {
        skyline::logger::s_Instance->LogFormat("[handleNnFsMountRom] Prevented game from double mounting as '%s'", RomMountName);
    }
    else {
        rc = nnFsMountRomImpl(path, buffer, size);
    }
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

void skyline_main() {
    // populate our own process handle
    Handle h;
    skyline::utils::Ipc::getOwnProcessHandle(&h);
    envSetOwnProcessHandle(h);
    svcBreak(0x69, 0,0);

    // init hooking setup
    //A64HookInit();

    // initialize logger
    /*skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    skyline::logger::s_Instance->Log("[skyline_main] Begining initialization.\n");
    skyline::logger::s_Instance->StartThread();*/

    // override exception handler to dump info
    /*nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, sizeof(exception_handler_stack),
                                    &exception_info);*/

    // hook to prevent the game from double mounting romfs
    /*A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), reinterpret_cast<void*>(handleNnFsMountRom),
                    (void**)&nnFsMountRomImpl);*/

    // manually init nn::ro ourselves, then stub it so the game doesn't try again
    /*nn::ro::Initialize();
    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), reinterpret_cast<void*>(stub), NULL);*/

    // hook abort to get crash info
    /*uintptr_t VAbort_ptr = 0;
    nn::ro::LookupSymbol(&VAbort_ptr, "_ZN2nn4diag6detail10VAbortImplEPKcS3_S3_iPKNS_6ResultEPKNS_2os17UserExceptionInfoES3_St9__va_list");
    A64HookFunction(reinterpret_cast<void*>(VAbort_ptr), reinterpret_cast<void*>(handleNnDiagDetailVAbortImpl), (void**)&VAbortImpl);*/

    // mount rom
    /*
    u64 cache_size = 0;
    Result rc;
    rc = nn::fs::QueryMountRomCacheSize(&cache_size);
    if(R_SUCCEEDED(rc)) {
        u8* cache = new u8[cache_size];
        rc = nnFsMountRomImpl(RomMountName, cache, cache_size);
        skyline::logger::s_Instance->LogFormat("[skyline_main] Mounted Rom (0x%x)", rc);
        if (R_FAILED(rc)) {
            delete[] cache;
        }
    }
    else {
        skyline::logger::s_Instance->LogFormat("[skyline_main] QueryMountRomCacheSize failed (0x%x)", rc);
    }*/

    /*skyline::logger::s_Instance->LogFormat("[skyline_main] text: 0x%" PRIx64 " | rodata: 0x%" PRIx64
                                           " | data: 0x%" PRIx64 " | bss: 0x%" PRIx64 " | heap: 0x%" PRIx64,
                                           skyline::utils::g_MainTextAddr, skyline::utils::g_MainRodataAddr,
                                           skyline::utils::g_MainDataAddr, skyline::utils::g_MainBssAddr,
                                           skyline::utils::g_MainHeapAddr);*/

    // start task queue
    /*skyline::utils::SafeTaskQueue* taskQueue = new skyline::utils::SafeTaskQueue(100);
    taskQueue->startThread(20, 3, 0x4000);
    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));*/

    // TODO: experiment more with NVN
    /*nvnInit(NULL);

    NVNdeviceBuilder deviceBuilder;
    nvnDeviceBuilderSetDefaults(&deviceBuilder);
    nvnDeviceBuilderSetFlags(&deviceBuilder, 0);

    NVNdevice device;
    nvnDeviceInitialize(&device, &deviceBuilder);

    nvnInit(&device); // re-init with our newly acquired device
    */
}

extern "C" void skyline_init() {
    skyline::utils::init();
    virtmemSetup();  // needed for libnx JIT

    skyline_main();
}
