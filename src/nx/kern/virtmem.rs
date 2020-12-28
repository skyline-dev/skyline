use crate::nx::kern::{svc, types::*};
use crate::nx::result::{*};

struct VirtualRegion {
    start: usize,
    end: usize,
}

impl VirtualRegion {
    pub fn in_region(self, addr: usize) -> bool {
        (addr >= self.start) && (addr < self.end)
    }
}
enum RegionType {
    Stack,
    Heap,
    LegacyAlias,
    Max
}

fn get_region_from_info(id0_addr: InfoType, id0_size: InfoType) -> Result<VirtualRegion, NxResult> {
    let base = svc::get_info(id0_addr, Handle::CURR_PROC, 0)? as usize;
    let size = svc::get_info(id0_size, Handle::CURR_PROC, 0)? as usize;

    Ok(VirtualRegion {start: base, end: base + size})
}

pub fn setup() -> Result<(), NxResult> {
    let address_space: VirtualRegion;
    let stack_region: VirtualRegion;

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
                    address_space   = VirtualRegion { start: 0x200000, end: 0x100000000     };
                    stack_region    = VirtualRegion { start: 0x200000, end: 0x40000000      };
                }
                Err(kern::INVALID_MEMORY_RANGE) => {
                    /* Invalid dst-address error means our 36-bit src-address was valid. */
                    /* Thus we are 36-bit. */
                    address_space   = VirtualRegion { start: 0x8000000, end: 0x1000000000   };
                    stack_region    = VirtualRegion { start: 0x8000000, end: 0x80000000     };
                }

                /* wut? */
                _ => unreachable!()
            }
        }
    }

    let heap_region =   get_region_from_info(InfoType::HeapRegionAddress, InfoType::HeapRegionSize)?;

    /* this can fail, just ignore */
    match get_region_from_info(InfoType::AliasRegionAddress, InfoType::AliasRegionSize) {
        Ok(region) => {
            todo!()
        }
        Err(_) => {}
    }

    todo!();
}

pub fn reserve(size: usize) -> Result<usize, NxResult> {
    todo!("Mutex");

    while true {
        todo!("impl");
    }

    Ok(0)
}