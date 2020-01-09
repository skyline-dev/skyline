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
    
    skyline::TcpLogger::LogFormat("Module \"%s\" loaded.", module->Name);

    return rc;
}

Result (*ogRoUnloadModule)(nn::ro::Module*);

Result roUnloadModuleHook(nn::ro::Module* module){

    skyline::TcpLogger::LogFormat("Module \"%s\" unloaded.", module->Name);

    return ogRoUnloadModule(module);
};

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

    skyline::TcpLogger::LogFormat("%s | 0x%x", path, *result);
};

skyline::arc::Hashes* hashes;

void stub() {}

void runtimePatchMain(void*){
    // wait for nnSdk to finish booting
    nn::os::SleepThread(nn::TimeSpan::FromSeconds(3));

    // init sd
    nn::fs::MountSdCardForDebug("sd");

    // init hooking setup
    A64HookInit();

    // find .text
    u64 text = memNextMapOfPerm((u64)nninitStartup, Perm_Rx); // nninitStartup can be reasonably assumed to be exported by main
    // find .rodata
    u64 rodata = memNextMapOfPerm((u64) nninitStartup, Perm_Rw);

    // find version string in memory
    const char* ver = "Ver. %d.%d.%d";
    size_t verLen = strlen(ver);
    char* verPtr = (char*) memmem((void*) rodata, INT64_MAX, (char*) ver, verLen);
    
    // write "Skyline" to found version string
    const char* skylineStr = "Skyline";
    size_t skylineStrLen = strlen(skylineStr);
    smcMemCpy(verPtr, (void*)skylineStr, skylineStrLen);
    smcWriteAddress8(&verPtr[skylineStrLen], 0); // write null terminator

    skyline::TcpLogger::StartThread(); // start logging thread

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
       
    hashes = new skyline::arc::Hashes();

    nn::ro::Initialize();
    A64HookFunction(
        reinterpret_cast<void*>(nn::ro::Initialize), 
        reinterpret_cast<void*>(stub), 
        NULL);

    skyline::Plugin::Manager::Init();
}

void skylineMain() {
    virtmemSetup();

    nn::os::CreateThread(&runtimePatchThread, runtimePatchMain, NULL, &runtimePatchStack, sizeof(runtimePatchStack), 20, 3);
    nn::os::StartThread(&runtimePatchThread);
}