#include "skyline/inlinehook/controlledpages.hpp"

#include "nn/os.hpp"
#include "skyline/utils/cpputils.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/arm/cache.h"
#include "skyline/nx/kernel/svc.h"
#include "skyline/nx/kernel/virtmem.h"
#include "skyline/nx/runtime/env.h"
#include "skyline/utils/utils.h"

#ifdef __cplusplus
}
#endif

#define PAGE_SIZE 0x1000

namespace skyline::inlinehook {

// mostly Atmosphere's libstratosphere
// TODO: this assumes sys ver v2.0.0+ish, could this support anything earlier?
struct AddressSpaceInfo {
    uintptr_t heap_base;
    size_t heap_size;
    uintptr_t heap_end;
    uintptr_t alias_base;
    size_t alias_size;
    uintptr_t alias_end;
    uintptr_t aslr_base;
    size_t aslr_size;
    uintptr_t aslr_end;
};

static constexpr uintptr_t AslrBase32Bit = 0x0000200000ul;
static constexpr size_t AslrSize32Bit = 0x003FE00000ul;
static constexpr uintptr_t AslrBase64BitDeprecated = 0x0008000000ul;
static constexpr size_t AslrSize64BitDeprecated = 0x0078000000ul;
static constexpr uintptr_t AslrBase64Bit = 0x0008000000ul;
static constexpr size_t AslrSize64Bit = 0x7FF8000000ul;

static Result getProcessAddressSpaceInfo(AddressSpaceInfo* out, Handle process_h) {
    /* Clear output. */
    memset(out, 0, sizeof(*out));

    /* Retrieve info from kernel. */
    R_TRY(svcGetInfo(&out->heap_base, InfoType_HeapRegionAddress, process_h, 0));
    R_TRY(svcGetInfo(&out->heap_size, InfoType_HeapRegionSize, process_h, 0));
    R_TRY(svcGetInfo(&out->alias_base, InfoType_AliasRegionAddress, process_h, 0));
    R_TRY(svcGetInfo(&out->alias_size, InfoType_AliasRegionSize, process_h, 0));

    R_TRY(svcGetInfo(&out->aslr_base, InfoType_AslrRegionAddress, process_h, 0));
    R_TRY(svcGetInfo(&out->aslr_size, InfoType_AslrRegionSize, process_h, 0));

    out->heap_end = out->heap_base + out->heap_size;
    out->alias_end = out->alias_base + out->alias_size;
    out->aslr_end = out->aslr_base + out->aslr_size;
    return 0;
}

static Result locateMappableSpaceModern(uintptr_t* out_address, size_t size) {
    MemoryInfo mem_info = {};
    u32 page_info = 0;
    uintptr_t cur_base = 0, cur_end = 0;

    AddressSpaceInfo address_space;
    R_TRY(getProcessAddressSpaceInfo(&address_space, CUR_PROCESS_HANDLE));
    cur_base = address_space.aslr_base;
    cur_end = cur_base + size;

    R_UNLESS(cur_base < cur_end, 0x10);

    s64 off;

    while (true) {
        off += sizeof(cur_base);

        if (address_space.heap_size &&
            (address_space.heap_base <= cur_end - 1 && cur_base <= address_space.heap_end - 1)) {
            /* If we overlap the heap region, go to the end of the heap region. */
            R_UNLESS(cur_base != address_space.heap_end, 0x104);
            cur_base = address_space.heap_end;
        } else if (address_space.alias_size &&
                   (address_space.alias_base <= cur_end - 1 && cur_base <= address_space.alias_end - 1)) {
            /* If we overlap the alias region, go to the end of the alias region. */
            R_UNLESS(cur_base != address_space.alias_end, 0x104);
            cur_base = address_space.alias_end;
        } else {
            R_ERRORONFAIL(svcQueryMemory(&mem_info, &page_info, cur_base));
            if (mem_info.type == 0 && mem_info.addr - cur_base + mem_info.size >= size) {
                *out_address = cur_base;
                return 0;
            }
            R_UNLESS(cur_base < mem_info.addr + mem_info.size, 0x104);

            cur_base = mem_info.addr + mem_info.size;
            R_UNLESS(cur_base < address_space.aslr_end, 0x104);
        }
        cur_end = cur_base + size;
        R_UNLESS(cur_base < cur_base + size, 0x104);
    }
}

static Result locateMappableSpace(uintptr_t* out_address, size_t size) {
    return locateMappableSpaceModern(out_address, size);
}

ControlledPages::ControlledPages(void* rx, size_t size) {
    this->rx = rx;
    this->size = size;
    isClaimed = false;
}

void ControlledPages::claim() {
    if (!isClaimed) {
        // get actual pages
        u64 alignedSrc = ALIGN_DOWN((u64)rx, PAGE_SIZE);
        size_t alignedSize = ALIGN_UP(size, PAGE_SIZE);

        // reserve space for rw pages
        u64 dst;
        R_ERRORONFAIL(locateMappableSpace(&dst, alignedSize));

        // map pages
        R_ERRORONFAIL(svcMapProcessMemory((void*)dst, envGetOwnProcessHandle(), alignedSrc, alignedSize));

        // provide rw pointer into the respective location in newly mapped pages
        rw = (void*)(dst + ((s64)rx - alignedSrc));
        isClaimed = true;

        // sanity check...
        if (*(u64*)rx != *(u64*)rw) {
            R_ERRORONFAIL(-1);
        }
    }
}

void ControlledPages::unclaim() {
    if (isClaimed) {
        // get actual pages
        u64 alignedSrc = ALIGN_DOWN((u64)rx, PAGE_SIZE);
        void* alignedDst = (void*)ALIGN_DOWN((u64)rw, PAGE_SIZE);
        size_t alignedSize = ALIGN_UP(size, PAGE_SIZE);

        // invalidate caches
        armDCacheFlush((void*)alignedDst, size);
        armICacheInvalidate((void*)alignedSrc, size);

        // unmap pages
        R_ERRORONFAIL(svcUnmapProcessMemory(alignedDst, envGetOwnProcessHandle(), alignedSrc, alignedSize));

        // clean up variables
        rw = NULL;
        isClaimed = false;
    }
}
};  // namespace skyline::inlinehook
