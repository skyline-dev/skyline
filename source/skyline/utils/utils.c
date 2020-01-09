#include "skyline/utils/utils.h"

u32 previousPowerOfTwo(u32 x) {
    if (x == 0) {
        return 0;
    }
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x - (x >> 1);
}

u64 memNextMap(u64 addr) {
    MemoryInfo info;
    u32 map;
    svcQueryMemory(&info, &map, addr);

    if(addr == info.addr  && info.type != MemType_Unmapped)
        return addr;

    return memNextMap(info.addr + info.size); 
}

u64 memNextMapOfType(u64 addr, u32 type){
    MemoryInfo info;
    u32 map;
    svcQueryMemory(&info, &map, addr);

    if(info.type == type)
        return info.addr;

    return memNextMapOfType(info.addr + info.size , type); 
}

u64 memNextMapOfPerm(u64 addr, u32 perm){
    MemoryInfo info;
    u32 map;
    svcQueryMemory(&info, &map, addr);

    if(info.perm == perm)
        return info.addr;

    return memNextMapOfType(info.addr + info.size , perm); 
}