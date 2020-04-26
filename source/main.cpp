#include "main.hpp"

#include "skyline/utils/ipc.hpp"
#include "skyline/logger/SdLogger.hpp"

nn::os::ThreadType runtimePatchThread;
char ALIGNA(0x1000) runtimePatchStack[0x7000];

// For handling exceptions
char ALIGNA(0x1000) exceptionHandlerStack[0x4000];
nn::os::UserExceptionInfo exceptionInfo;

void exceptionHandler(nn::os::UserExceptionInfo* info){
    
    skyline::logger::s_Instance->LogFormat("Exception occured!\n");

    skyline::logger::s_Instance->LogFormat("Error description: %x\n", info->ErrorDescription);
    for(int i = 0; i < 29; i++)
        skyline::logger::s_Instance->LogFormat("X[%02i]: %" PRIx64  "\n", i, info->CpuRegisters[i].x);
    skyline::logger::s_Instance->LogFormat("FP: %" PRIx64  "\n", info->FP.x);
    skyline::logger::s_Instance->LogFormat("LR: %" PRIx64 " \n", info->LR.x);
    skyline::logger::s_Instance->LogFormat("SP: %" PRIx64   "\n", info->SP.x);
    skyline::logger::s_Instance->LogFormat("PC: %" PRIx64  "\n", info->PC.x);

    //*((u64*)0) = 0x69;
}

void stub() {}

Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size){
    Result rc = nnFsMountRomImpl(path, buffer, size);
    skyline::logger::s_Instance->LogFormat("[handleNnFsMountRom] Mounted ROM (0x%x)", rc);
    skyline::utils::g_RomMountStr = std::string(path) + ":/";
    nn::os::SignalEvent(&skyline::utils::g_RomMountedEvent);
    return rc;
}

void logReport(nn::prepo::PlayReport* report){
    nn::fs::CreateDirectory("sd:/prepo");
    nn::time::PosixTime time;
    nn::time::StandardUserSystemClock::GetCurrentTime(&time);
    std::string reportName = "sd:/prepo/";
    reportName += std::string((char*)&report->m_EventName);
    reportName += " ";
    reportName += std::to_string(time.time);
    reportName += ".bin";

    skyline::utils::writeFile(reportName.c_str(), 0, report->m_Buff, report->m_End);
}

Result (*nnPrepoSaveImpl)(nn::prepo::PlayReport*);
Result handleNnPrepoSave(nn::prepo::PlayReport* report) {
    skyline::logger::s_Instance->LogFormat("[nnPrepoSaveImpl] Report sent: %s", &report->m_EventName);
    logReport(report);

    return nnPrepoSaveImpl(report);
}

Result (*nnPrepoSaveWUidImpl)(nn::prepo::PlayReport*, nn::account::Uid const&);
Result handleNnPrepoSaveWUid(nn::prepo::PlayReport* report, nn::account::Uid const& acc) {
    skyline::logger::s_Instance->LogFormat("[handleNnPrepoSaveWUid] Report sent: %s", &report->m_EventName);
    logReport(report);
    
    return nnPrepoSaveWUidImpl(report, acc);
}

u32 (*murmurHashImpl)(char*, u32);
u32 handleMurmurHash(char* input, u32 seed) {
    std::string str(input);
    u32 hash = murmurHashImpl(input, seed);
    if(str != "invalid_name" && str != "" && str != "InvalidStage"){
        //skyline::logger::s_Instance->LogFormat("[handleMurmurHash] %s | %x = %x", input, seed, hash);
    }

    return hash;
}

u32 (*crcHashImpl)(char const*);
u32 handleCrcHash(char const* str) {
    u32 hash = crcHashImpl(str);

    if(str != NULL)
        skyline::logger::s_Instance->LogFormat("[handleCrcHash] %s = %x", str, hash);

    return hash;
}

struct L2CAgent {
    uint64_t vtable;
    uint64_t lua_state_agent;
    uint64_t unk10;
    uint64_t unk18;
    uint64_t unk20;
    uint64_t unk28;
    uint64_t unk30;
    uint64_t unk38;
    uint64_t lua_state_agentbase;
};

#define LOAD64 *(u64*)

extern void frame(u64, float) asm("_ZN3app10sv_animcmd5frameEP9lua_Statef");
extern bool is_excute(u64) asm("_ZN3app10sv_animcmd9is_excuteEP9lua_State");
u64 squirtle_ftilt(L2CAgent* l2c_agent, void* variadic) {
    frame(l2c_agent->lua_state_agent, 5.0);
    if (is_excute(l2c_agent->lua_state_agent)) {
        skyline::logger::s_Instance->Log("[Squirtle Utilt] Frame 5\n");
    }

    frame(l2c_agent->lua_state_agent, 10.0);
    if (is_excute(l2c_agent->lua_state_agent)) {
        skyline::logger::s_Instance->Log("[Squirtle Utilt] Frame 10\n");
    }
}

extern u64 sv_set_function_hash(L2CAgent*, u64 (*func)(L2CAgent*, void*), u64 hash) asm("_ZN3lib8L2CAgent20sv_set_function_hashEPvN3phx6Hash40E");
void replace_scripts(L2CAgent* l2c_agent, u8 category, int kind) {
    // fighter
    if (category == 0) {//BATTLE_OBJECT_CATEGORY_FIGHTER) {
        // squirtle
        if (kind == 36) {//FIGHTER_KIND_PZENIGAME) {
            sv_set_function_hash(
                l2c_agent,
                &squirtle_ftilt,
                0xEF3BCA119); // hash40("game_attackhi3")
        }
    }
}

extern clear_lua_stack(L2CAgent* l2c_agent) asm("_ZN3lib8L2CAgent15clear_lua_stackEv");
u64 (*clear_lua_stack_impl)(L2CAgent*);
u64 clear_lua_stack_replace(L2CAgent* l2c_agent) {
    u64 lua_state = l2c_agent->lua_state_agent;
    if ((lua_state - 8) && 
        LOAD64(lua_state - 8) &&
        (LOAD64(LOAD64(lua_state - 8) + 416LL))) {
        u8 battle_object_category = *(u8*)(LOAD64(lua_state - 8) + 404LL);
        int battle_object_kind = *(int*)(LOAD64(lua_state - 8) + 408LL);
        replace_scripts(l2c_agent, battle_object_category, battle_object_kind);
    }

    return clear_lua_stack_impl(l2c_agent);
}

void runtimePatchMain(void*){
    // init sd
    Result rc = nn::fs::MountSdCardForDebug("sd");
    // create temp directory
    nn::fs::CreateDirectory("sd:/tmp");
    // initalize logger to SD 
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();

    skyline::logger::s_Instance->LogFormat("[runtimePatchMain] Mounted SD (0x%x)", rc);

    skyline::logger::s_Instance->Log("[runtimePatchMain] Begining initialization.\n");

    // swap out to Tcp
    delete skyline::logger::s_Instance;
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();

    // ask pm for my own process handle
    Handle h;
    skyline::utils::Ipc::getOwnProcessHandle(&h);
    envSetOwnProcessHandle(h);

    // init hooking setup
    A64HookInit();

    // hook rom mounting to signal that it has occured
    nn::os::InitializeEvent(&skyline::utils::g_RomMountedEvent, false, nn::os::EventClearMode_AutoClear);
    A64HookFunction(
        reinterpret_cast<void*>(nn::fs::MountRom),
        reinterpret_cast<void*>(handleNnFsMountRom),
        (void**) &nnFsMountRomImpl
    );

    A64HookFunction(
        reinterpret_cast<void*>(clear_lua_stack),
        reinterpret_cast<void*>(clear_lua_stack_replace),
        (void**) &clear_lua_stack_impl
    );

    skyline::logger::s_Instance->StartThread(); // start logging thread

    // wait for rom to become available
    /*while(!nn::fs::CanMountRomForDebug()){
        nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(10000000));
    }*/


    /*
    u64 romCacheSize;
    Result rc = nn::fs::QueryMountRomCacheSize(&romCacheSize);
    skyline::TcpLogger::LogFormat("[runtimePatchMain] Queried required ROM cache size: 0x%x | (0x%x)", romCacheSize, rc);
    void* romCache = malloc(romCacheSize);
    rc = nn::fs::MountRom("rom", romCache, romCacheSize);
    skyline::TcpLogger::LogFormat("[runtimePatchMain] Mounted ROM (0x%x)", rc);
    */


    skyline::logger::s_Instance->LogFormat("[runtimePatchMain] text: 0x%" PRIx64 " | rodata: 0x%" PRIx64 " | data: 0x%" PRIx64 " | bss: 0x%" PRIx64 " | heap: 0x%" PRIx64, 
        skyline::utils::g_MainTextAddr,
        skyline::utils::g_MainRodataAddr,
        skyline::utils::g_MainDataAddr,
        skyline::utils::g_MainBssAddr,
        skyline::utils::g_MainHeapAddr
    );

    skyline::utils::SafeTaskQueue *taskQueue = new skyline::utils::SafeTaskQueue(100);
    taskQueue->startThread(20, 3, 0x4000);

    // override exception handler to dump info 
    nn::os::SetUserExceptionHandler(exceptionHandler, exceptionHandlerStack, sizeof(exceptionHandlerStack), &exceptionInfo);

    Result (nn::prepo::PlayReport::*saveImpl)() = &nn::prepo::PlayReport::Save;
    Result (nn::prepo::PlayReport::*saveWUidImpl)(nn::account::Uid const&) = &nn::prepo::PlayReport::Save;
    /*A64HookFunction(
        reinterpret_cast<void*>(saveImpl), 
        reinterpret_cast<void*>(handleNnPrepoSave), 
        (void**) &nnPrepoSaveImpl
    );
    A64HookFunction(
        reinterpret_cast<void*>(saveWUidImpl), 
        reinterpret_cast<void*>(handleNnPrepoSaveWUid), 
        (void**) &nnPrepoSaveWUidImpl
    );
    A64HookFunction(
        reinterpret_cast<void*>(skyline::utils::g_MainTextAddr + 0x1C20),
        reinterpret_cast<void*>(handleMurmurHash),
        (void**) &murmurHashImpl
    );
    A64HookFunction(
        reinterpret_cast<void*>(skyline::utils::g_MainTextAddr + 0x01A4BCA0),
        reinterpret_cast<void*>(handleCrcHash),
        (void**) &crcHashImpl
    );*/
            

    skyline::utils::Task* initHashesTask = new skyline::utils::Task {
        []() {
            // wait for ROM to be mounted
            if(!nn::os::TimedWaitEvent(&skyline::utils::g_RomMountedEvent, nn::TimeSpan::FromSeconds(10))) {
                //skyline::TcpLogger::SendRawFormat("[ROM Waiter] Missed ROM mount event!\n");
            }

            Result (*nnRoInitializeImpl)();
            A64HookFunction(
                reinterpret_cast<void*>(nn::ro::Initialize), 
                reinterpret_cast<void*>(stub), 
                (void**) &nnRoInitializeImpl
            );
            nnRoInitializeImpl();
            skyline::plugin::Manager::Init();
        }
    };

    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(initHashesTask));


    //smcWriteAddress32(reinterpret_cast<void*>(text + 0x012775C8), 0x26000014); // NOP
    //smcWriteAddress32(reinterpret_cast<void*>(text + 0x012775B0), 0x1F2003D5); 
    //smcWriteAddress32(reinterpret_cast<void*>(text + 0x01277538), 0x1F2003D5);


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

extern "C" void skylineMain() {
    skyline::utils::init();
    virtmemSetup();

    runtimePatchMain(NULL);

    //nn::os::CreateThread(&runtimePatchThread, runtimePatchMain, NULL, &runtimePatchStack, sizeof(runtimePatchStack), 20, 2);
    //nn::os::StartThread(&runtimePatchThread);
}
