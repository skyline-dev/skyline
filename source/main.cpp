#include "main.hpp"

extern "C" void __custom_init(void) {  skylineMain(); }

// DT_FINI here for completeness.
extern "C" void __custom_fini(void) {}

nn::os::ThreadType runtimePatchThread;
char __attribute__((aligned(0x1000))) runtimePatchStack[0x6000];

// unused in the context of NSOs
extern "C" void skylineInit(void* ctx, Handle main_thread, LoaderReturnFn saved_lr){
    *((u64*)0) = 0x69;
}

void runtimePatchMain(void*){
    // wait for nnSdk to finish booting
    nn::os::SleepThread(nn::TimeSpan::FromSeconds(3));

    // init sd
    nn::fs::MountSdCardForDebug("sd");

    // init hooking setup
    A64HookInit();

    // find .rodata
    u64 rodata = memNextMapOfPerm((u64) nninitStartup, Perm_Rw); // nninitStartup can be reasonably assumed to be exported by main

    char* ver = "Ver. %d.%d.%d";
    size_t verLen = strlen(ver);
    char* verPtr = (char*) memmem((void*) rodata, INT64_MAX, ver, verLen);
    
    char* skylineStr = "Skyline";
    size_t skylineStrLen = strlen(skylineStr);

    smcMemCpy(verPtr, skylineStr, skylineStrLen);
    smcWriteAddress8(&verPtr[skylineStrLen], 0);

    skyline::TcpLogger::StartThread();
}

void skylineMain() {
    virtmemSetup();

    nn::os::CreateThread(&runtimePatchThread, runtimePatchMain, NULL, &runtimePatchStack, sizeof(runtimePatchStack), 20, 3);
    nn::os::StartThread(&runtimePatchThread);
}