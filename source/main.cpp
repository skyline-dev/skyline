#include "main.hpp"

extern "C" {
    void __custom_init(void) {  skylineMain(); }
    void __custom_fini(void) {}

    void *__dso_handle; // for linking with libc++

    // unused in the context of NSOs
    extern "C" void skylineInit(void* ctx, Handle main_thread, LoaderReturnFn saved_lr){
        *((u64*)0) = 0x69;
    }
}

nn::os::ThreadType runtimePatchThread;
char ALIGNA(0x1000) runtimePatchStack[0x7000];

// For handling exceptions
char ALIGNA(0x1000) exceptionHandlerStack[0x4000];
nn::os::UserExceptionInfo exceptionInfo;

Result (*ogRoLoadModule)(nn::ro::Module*, const void *, void *, ulong, int);

Result roLoadModuleHook(nn::ro::Module* module, const void * nro, void * buffer, ulong bufferSize, int unk) {
    Result rc = ogRoLoadModule(module, nro, buffer, bufferSize, unk);
    
    skyline::TcpLogger::LogFormat("[roLoadModuleHook] Module \"%s\" loaded.", module->Name);

    return rc;
}

Result (*ogRoUnloadModule)(nn::ro::Module*);

Result roUnloadModuleHook(nn::ro::Module* module){

    skyline::TcpLogger::LogFormat("[roUnloadModuleHook] Module \"%s\" unloaded.", module->Name);

    return ogRoUnloadModule(module);
}

void exceptionHandler(nn::os::UserExceptionInfo* info){
    
    skyline::TcpLogger::SendRaw("Exception occured!\n");

    skyline::TcpLogger::SendRawFormat("Error description: %x\n", info->ErrorDescription);
    for(int i = 0; i < 29; i++)
        skyline::TcpLogger::SendRawFormat("X[%02i]: %" PRIx64  "\n", i, info->CpuRegisters[i].x);
    skyline::TcpLogger::SendRawFormat("FP: %" PRIx64  "\n", info->FP.x);
    skyline::TcpLogger::SendRawFormat("LR: %" PRIx64 " \n", info->LR.x);
    skyline::TcpLogger::SendRawFormat("SP: %" PRIx64   "\n", info->SP.x);
    skyline::TcpLogger::SendRawFormat("PC: %" PRIx64  "\n", info->PC.x);

    //*((u64*)0) = 0x69;
}

void (*lookupCharacterFile)(uint*, char*);

void lookupCharacterFileHook(uint* result, char* path){
    lookupCharacterFile(result, path);

    skyline::TcpLogger::LogFormat("%s | 0x%x\n", path, *result);
}



void stub() {}

Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size){
    Result rc = nnFsMountRomImpl(path, buffer, size);
    skyline::TcpLogger::LogFormat("[handleNnFsMountRom] Mounted ROM (0x%x)", rc);
    skyline::utils::g_RomMountStr = std::string(path) + ":/";
    nn::os::SignalEvent(&skyline::utils::g_RomMountedEvent);
    return rc;
}

void runtimePatchMain(void*){
    // wait for nnSdk to finish booting
    nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(130000000));

    skyline::TcpLogger::Log("[runtimePatchMain] Begining initialization.");

    // init hooking setup
    A64HookInit();

    // hook rom mounting to signal that it has occured
    nn::os::InitializeEvent(&skyline::utils::g_RomMountedEvent, false, nn::os::EventClearMode_AutoClear);
    A64HookFunction(
        reinterpret_cast<void*>(nn::fs::MountRom),
        reinterpret_cast<void*>(handleNnFsMountRom),
        (void**) &nnFsMountRomImpl
    );

    skyline::TcpLogger::StartThread(); // start logging thread

    // init sd
    Result rc = nn::fs::MountSdCardForDebug("sd");
    skyline::TcpLogger::LogFormat("[runtimePatchMain] Mounted SD (0x%x)", rc);

    skyline::TcpLogger::LogFormat("[runtimePatchMain] text: 0x%" PRIx64 " | rodata: 0x%" PRIx64 " | data: 0x%" PRIx64 " | bss: 0x%" PRIx64 " | heap: 0x%" PRIx64, 
        skyline::utils::g_MainTextAddr,
        skyline::utils::g_MainRodataAddr,
        skyline::utils::g_MainDataAddr,
        skyline::utils::g_MainBssAddr,
        skyline::utils::g_MainHeapAddr
    );

    skyline::utils::SafeTaskQueue *taskQueue = new skyline::utils::SafeTaskQueue(100);
    taskQueue->startThread(20, 3, 0x4000);

    skyline::utils::Task* patchVersionTask = new skyline::utils::Task {
        [](){

    // find version string in memory
    const char* ver = "Ver. %d.%d.%d";
    size_t verLen = strlen(ver);
            char* verPtr = (char*) memmem((void*) skyline::utils::g_MainRodataAddr, skyline::utils::g_MainDataAddr - skyline::utils::g_MainRodataAddr, (char*) ver, verLen);
    
    // write "Skyline" to found version string
    const char* skylineStr = "Skyline";
    size_t skylineStrLen = strlen(skylineStr);
            smcMemCpy(verPtr, (void*)skylineStr, skylineStrLen+1); //+1 to write null terminator
        }
    };
    
    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(patchVersionTask));

    // override exception handler to dump info 
    nn::os::SetUserExceptionHandler(exceptionHandler, exceptionHandlerStack, sizeof(exceptionHandlerStack), &exceptionInfo);

    // nn::ro hooks
    A64HookFunction(
        reinterpret_cast<void*>(nn::ro::LoadModule),
        reinterpret_cast<void*>(roLoadModuleHook), 
        (void**) &ogRoLoadModule);
    A64HookFunction(
        reinterpret_cast<void*>(nn::ro::UnloadModule), 
        reinterpret_cast<void*>(roUnloadModuleHook), 
        (void**)&ogRoUnloadModule);

    // data.arc interception hooks
    A64HookFunction(
        reinterpret_cast<void*>(text + 0x3126030), 
        reinterpret_cast<void*>(lookupCharacterFileHook), 
        (void**) &lookupCharacterFile);
       

    skyline::utils::Task* initHashesTask = new skyline::utils::Task {
        []() {
            // wait for ROM to be mounted
            if(!nn::os::TimedWaitEvent(&skyline::utils::g_RomMountedEvent, nn::TimeSpan::FromSeconds(5))) {
                skyline::TcpLogger::SendRawFormat("[ROM Waiter] Missed ROM mount event!\n");
            }

            skyline::utils::g_Hashes = new skyline::arc::Hashes();

            Result (*nnRoInitializeImpl)();
    A64HookFunction(
        reinterpret_cast<void*>(nn::ro::Initialize), 
        reinterpret_cast<void*>(stub), 
                (void**) &nnRoInitializeImpl);
            nnRoInitializeImpl();

            skyline::plugin::Manager::Init();
        }
    };

    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(initHashesTask));




    // crashes this early in init...
    /*nvnInit(NULL);

    NVNdeviceBuilder deviceBuilder;
    nvnDeviceBuilderSetDefaults(&deviceBuilder);
    nvnDeviceBuilderSetFlags(&deviceBuilder, 0);

    NVNdevice device;
    nvnDeviceInitialize(&device, &deviceBuilder);

    nvnInit(&device); // re-init with our newly aquired device
    */
}

void skylineMain() {
    skyline::utils::populateMainAddrs();
    virtmemSetup();

    nn::os::CreateThread(&runtimePatchThread, runtimePatchMain, NULL, &runtimePatchStack, sizeof(runtimePatchStack), 20, 3);
    nn::os::StartThread(&runtimePatchThread);
}