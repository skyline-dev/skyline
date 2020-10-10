#include "main.hpp"

#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/ipc.hpp"

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
    // wait for ROM to be mounted
    if (!nn::os::TimedWaitEvent(&skyline::utils::g_RomMountedEvent, nn::TimeSpan::FromSeconds(10))) {
        skyline::logger::s_Instance->SendRawFormat("[ROM Waiter] Missed ROM mount event!");
    }

    // mount sd
    Result rc = nn::fs::MountSdCardForDebug("sd");
    skyline::logger::s_Instance->LogFormat("[skyline_main] Mounted SD (0x%x)", rc);

    // load plugins
    skyline::plugin::Manager::LoadPlugins();
}};

void stub() {}

Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = nnFsMountRomImpl(path, buffer, size);
    skyline::logger::s_Instance->LogFormat("[handleNnFsMountRom] Mounted ROM (0x%x)", rc);
    skyline::utils::g_RomMountStr = std::string(path) + ":/";
    nn::os::SignalEvent(&skyline::utils::g_RomMountedEvent);
    nn::os::WaitEvent(&after_romfs_task->completionEvent);
    return rc;
}

Result handleNnDiagDetailAbortImpl(char const* str1, char const* str2, char const* str3, s32 code) {
    const char* fmt_str = "HOS is aborting with:\n%s\n%s\n%s\nCode: 0x%x";
    size_t len = snprintf(NULL, 0, fmt_str, str1, str2, str3);
    char* ptr = new char[len + 2];
    memset(ptr, 0, len + 2);
    snprintf(ptr, len + 1, fmt_str, str1, str2, str3);
    ptr[len] = '\0';
    skyline::logger::s_Instance->LogFormat(ptr);
    nn::err::ApplicationErrorArg* error =
        new nn::err::ApplicationErrorArg(69, "The software has closed due to an error.", ptr,
                                         nn::settings::LanguageCode::Make(nn::settings::Language::Language_English));

    nn::err::ShowApplicationError(*error);
    return 0;
}

void skyline_main() {
    // populate our own process handle
    Handle h;
    skyline::utils::Ipc::getOwnProcessHandle(&h);
    envSetOwnProcessHandle(h);

    // init hooking setup
    A64HookInit();

    // initialize logger
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    skyline::logger::s_Instance->Log("[skyline_main] Begining initialization.\n");
    skyline::logger::s_Instance->StartThread();

    // override exception handler to dump info
    nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, sizeof(exception_handler_stack),
                                    &exception_info);

    // hook abort to get crash info
    void (*AbortImpl)(char const*, char const*, char const*, s32);
    AbortImpl = nn::diag::detail::AbortImpl;
    A64HookFunction(reinterpret_cast<void*>(AbortImpl), reinterpret_cast<void*>(handleNnDiagDetailAbortImpl), NULL);

    // hook to get a signal upon rom mount
    nn::os::InitializeEvent(&skyline::utils::g_RomMountedEvent, false, nn::os::EventClearMode_AutoClear);
    A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), reinterpret_cast<void*>(handleNnFsMountRom),
                    (void**)&nnFsMountRomImpl);

    // manually init nn::ro ourselves, then stub it so the game doesn't try again
    nn::ro::Initialize();
    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), reinterpret_cast<void*>(stub), NULL);

    skyline::logger::s_Instance->LogFormat("[skyline_main] text: 0x%" PRIx64 " | rodata: 0x%" PRIx64
                                           " | data: 0x%" PRIx64 " | bss: 0x%" PRIx64 " | heap: 0x%" PRIx64,
                                           skyline::utils::g_MainTextAddr, skyline::utils::g_MainRodataAddr,
                                           skyline::utils::g_MainDataAddr, skyline::utils::g_MainBssAddr,
                                           skyline::utils::g_MainHeapAddr);

    // start task queue
    skyline::utils::SafeTaskQueue* taskQueue = new skyline::utils::SafeTaskQueue(100);
    taskQueue->startThread(20, 3, 0x4000);
    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));

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
