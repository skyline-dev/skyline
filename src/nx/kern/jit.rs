use crate::nx::kern::types::{Handle, CodeMapOperation, MemoryPermission, PAGE_SIZE};
use crate::nx::kern::svc;
use crate::nx::kern::detect;
use crate::nx::kern::virtmem;
use crate::nx::arm::cache;
use crate::nx::result::NxResult;
use crate::nx::util::cur_proc_handle;
use std::alloc::{alloc, dealloc, Layout};

#[derive(PartialEq, Eq)]
pub enum JitType {
    CodeMemory,
    JitMemory,
}

pub struct Jit {
    jtype: JitType,
    size: usize,
    src_addr: usize,
    rx_addr: usize,
    rw_addr: usize,
    is_executable: bool,
    handle: Handle,
}

impl Jit {

    fn attempt_map(&mut self) -> Result<(), NxResult> {
        match self.jtype {

            /* We will be swapping out RW/RX on demand.  */
            JitType::CodeMemory => {

                /* The RW will be the heap allocated area. */
                self.rw_addr = self.src_addr;
            }

            /* We will create a double mapping, RX and RX. */
            JitType::JitMemory => {

                /* Find some unmapped space for our RW pages to reside. */
                self.rw_addr = virtmem::reserve(self.size)?;

                /* Create code memory with heap area. */
                self.handle = svc::create_code_memory(self.src_addr, self.size)?;

                /* Mark our RW pages as the owner memory. */
                svc::control_code_memory(self.handle, CodeMapOperation::MapOwner, self.rw_addr, self.size, MemoryPermission::READ_WRITE)?;

                /* Mark our RX pages as the slave memory.*/
                svc::control_code_memory(self.handle, CodeMapOperation::MapSlave, self.rx_addr, self.size, MemoryPermission::READ_EXECUTE)?;
            }
        }

        Ok(())
    }

    pub fn init(&mut self, mut buf: usize, size: usize) -> Result<(), NxResult> {

        /* 4.0.0 introduced code memory, which allows owner/slave JIT (owner process has RW access, slave process has RX access) */
        /* 5.0.0+ restricts that the owner process cannot be equal to the slave process, but we can check if the kernel is patched to support this anyway. */
        /* If we cannot take advantage of code memory for one reason or another, fallback to reprotecting the memory on demand. */
        /* NOTE: We assume all svcs are allowed here. */
        if detect::version_above_400() && (!detect::version_above_500() || detect::has_kernel_patch()) {
            self.jtype = JitType::JitMemory;
        } else {
            self.jtype = JitType::CodeMemory;
        }

        self.size = (size + (PAGE_SIZE - 1)) & !(PAGE_SIZE - 1);

        /* If provided buf is null, find some free space with virtmem. */
        if buf == 0 {
            buf = virtmem::reserve(self.size)?.into();
        }

        /* We need heap area to sacrifice for mapping. (page aligned) */
        let layout = Layout::from_size_align(self.size, PAGE_SIZE).unwrap();
        unsafe {
            self.src_addr = alloc(layout) as usize;
        }
        
        self.rx_addr = buf;
        self.handle = Handle::INVALID;
        self.is_executable = false;

        /* Attempt to init the necessary mappings for JIT, cleaning up on failure. */
        self.attempt_map().map_err(|res| -> NxResult {
            
            /* Cleanup handle if code memory was made. */
            if self.handle != Handle::INVALID {
                svc::close_handle(self.handle).unwrap();
                self.handle = Handle::INVALID;
            }
            
            /* Free up heap area. */
            unsafe {
                dealloc(self.src_addr as *mut u8, layout);
                self.src_addr = 0;
            }

            res
        })?;


        Ok(())
    }
    
    pub fn to_writable(&mut self) -> Result<(), NxResult> {
        if !self.is_executable {
            /* There's nothing to do... */
            return Ok(());
        }

        match self.jtype {
            JitType::CodeMemory => {
                /* Unmap code, re-mapping the heap area. */
                svc::unmap_process_code_memory(cur_proc_handle::get(), self.rx_addr, self.src_addr, self.size)?;
            },

            JitType::JitMemory => {
                /* Both RW and RX pages exist at the same time, there's nothing to do. */
            }
        }

        self.is_executable = false;

        Ok(())
    }

    pub fn to_executable(&mut self) -> Result<(), NxResult> {

        if self.is_executable {
            /* There's nothing to do... */
            return Ok(());
        }
        
        match self.jtype {
            JitType::CodeMemory => {

                /* Map heap area as code to unmapped memory. This unmaps the heap area in the process. */
                svc::map_process_code_memory(cur_proc_handle::get(), self.rx_addr, self.src_addr, self.size)?;
                
                /* Reprotect new mapping as RX. */
                svc::set_process_memory_permission(cur_proc_handle::get(), self.rx_addr, self.size, MemoryPermission::READ_EXECUTE).map_err(|err| -> NxResult {
                    
                    /* Go back to writable if failure. */
                    self.to_writable().unwrap();
                    err
                })?;
            },

            JitType::JitMemory => {

                /* Both RW and RX pages exist at the same time. We just need to inform the instruction/data caches. */
                cache::flush_data(self.rw_addr, self.size);
                cache::invalidate_instruction(self.rx_addr, self.size);
            }
        }

        self.is_executable = true;

        Ok(())
    }
}

impl Drop for Jit {
    fn drop(&mut self) {
        match self.jtype {
            JitType::CodeMemory => {
                self.to_writable().unwrap();
            },

            JitType::JitMemory => {
                /* Unmap code memory pages. */
                svc::control_code_memory(self.handle, CodeMapOperation::UnmapOwner, self.rw_addr, self.size, MemoryPermission::NONE).unwrap();
                svc::control_code_memory(self.handle, CodeMapOperation::UnmapSlave, self.rw_addr, self.size, MemoryPermission::NONE).unwrap();

                /* Delete code memory. */
                svc::close_handle(self.handle).unwrap();
            }
        }

        /* Free heap area. */
        let layout = Layout::from_size_align(self.size, PAGE_SIZE).unwrap();
        unsafe {
            dealloc(self.src_addr as *mut u8, layout);
        }
        self.src_addr = 0;
    }
}