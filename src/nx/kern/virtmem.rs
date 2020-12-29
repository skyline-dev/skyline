use crate::nx::kern::{svc, types::*};
use crate::nx::result::*;
use std::sync::Mutex;
use std::ops::Range;


fn get_region_from_info(id0_addr: InfoType, id0_size: InfoType) -> Result<Range<usize>, NxResult> {
    let base = svc::get_info(id0_addr, Handle::CURR_PROC, 0)? as usize;
    let size = svc::get_info(id0_size, Handle::CURR_PROC, 0)? as usize;

    Ok(Range {start: base, end: base + size})
}

enum RegionType {
    Stack,
    Heap,
    LegacyAlias,
    Max
}

struct VirtmemState {
    address_space: Range<usize>,
    regions: [Range<usize>; RegionType::Max as usize],
    current_addr: usize,
    current_map_addr: usize,
}

impl VirtmemState {
    fn new() -> Result<Self, NxResult> {
        let address_space: Range<usize>;
        let stack_region: Range<usize>;

        match get_region_from_info(InfoType::AslrRegionAddress, InfoType::AslrRegionSize) {
            Ok(_address_space) => {
                address_space = _address_space;
                stack_region = get_region_from_info(InfoType::StackRegionAddress, InfoType::StackRegionSize)?;
            },
            Err(_) => {
                match svc::unmap_memory(0xFFFFFFFFFFFFE000, 0xFFFFFE000, 0x1000) {
                    Err(kern::INVALID_MEMORY_STATE) => {
                        /* Invalid src-address error means that a valid 36-bit address was rejected. */
                        /* Thus we are 32-bit. */
                        address_space   = 0x200000..0x100000000;
                        stack_region    = 0x200000..0x40000000;
                    }
                    Err(kern::INVALID_MEMORY_RANGE) => {
                        /* Invalid dst-address error means our 36-bit src-address was valid. */
                        /* Thus we are 36-bit. */
                        address_space   = 0x8000000..0x1000000000;
                        stack_region    = 0x8000000..0x80000000;
                    }
    
                    /* wut? */
                    _ => unreachable!()
                }
            }
        }
    
        let heap_region= get_region_from_info(InfoType::HeapRegionAddress, InfoType::HeapRegionSize)?;

        let alias_region =  match get_region_from_info(InfoType::AliasRegionAddress, InfoType::AliasRegionSize) {
            Ok(region) => region,
            Err(_) => 0..0 /* On failure, initialize entry as empty */
        };

        Ok(Self {
            address_space,
            regions: [
                stack_region,
                heap_region,
                alias_region,
            ],
            current_addr: 0,
            current_map_addr: 0,
        })
    }
}

lazy_static! {
    static ref VIRTMEM_STATE: Mutex<VirtmemState> = Mutex::new(VirtmemState::new().unwrap());
}

pub fn reserve(size: usize) -> Result<usize, NxResult> {
    let mut state = VIRTMEM_STATE.lock().unwrap();

    /* Align size by pages */
    let aligned_size = (size + (PAGE_SIZE - 1)) & !(PAGE_SIZE - 1);

    let mut addr = state.current_addr;

    loop {
        /* Add a guard page. */
        addr += PAGE_SIZE;

        /* Check if we are within the address space. */
        if state.address_space.contains(&addr) {
            /* Wrap around to the start of the address space. */
            addr = state.address_space.start;
        }

        let res = svc::query_memory(addr)?;
        let meminfo = res.meminfo;

        /* Ensure this memory area isn't taken */
        if meminfo.state != MemoryState::Free {
            /* This is taken, move past it. */
            addr = meminfo.base_address + meminfo.size;
            continue;
        }

        /* Ensure this area can fit the amount we want. */
        if addr + aligned_size > meminfo.base_address + meminfo.size {
            /* We can't fit here, move past it. */
            addr = meminfo.base_address + meminfo.size;
            continue;
        }

        let end = addr + aligned_size - 1;

        /* See if the region intersects anything. */
        let intersected_region = state.regions.iter()
            .filter(|reg| reg.contains(&addr) || reg.contains(&end))
            .next();

        match intersected_region {
            /* We intersected a region, move past it. */
            Some(region) => addr = region.end,
            /* Address looks good, we can break. */
            None => break,
        }
    }

    state.current_addr = addr + aligned_size;

    Ok(addr)
}

// TODO: reserve_stack