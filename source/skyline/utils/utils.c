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

Result memGetMap(MemoryInfo* info, u64 addr) {
    u32 map;
    return svcQueryMemory(info, &map, addr);
}

u64 memGetMapAddr(u64 addr) {
    MemoryInfo map;
    memGetMap(&map, addr);
    return map.addr;
}

u64 memNextMap(u64 addr) {
    MemoryInfo map;
    memGetMap(&map, addr);
    memGetMap(&map, map.addr + map.size);

    if (map.type != MemType_Unmapped) return map.addr;

    return memNextMap(map.addr);
}

u64 memNextMapOfType(u64 addr, u32 type) {
    MemoryInfo map;
    memGetMap(&map, addr);
    memGetMap(&map, map.addr + map.size);

    if (map.type == type) return map.addr;

    return memNextMapOfType(map.addr, type);
}

u64 memNextMapOfPerm(u64 addr, u32 perm) {
    MemoryInfo map;
    memGetMap(&map, addr);
    memGetMap(&map, map.addr + map.size);

    if (map.perm == perm) return map.addr;

    return memNextMapOfType(map.addr, perm);
}

u64 get_program_id() {
    u64 program_id;
    svcGetInfo(&program_id, 18, CUR_PROCESS_HANDLE, 0);
    return program_id;
}

