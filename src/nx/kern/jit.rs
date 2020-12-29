use crate::nx::kern::types::{Handle, PAGE_SIZE};
use crate::nx::kern::detect;
use crate::nx::result::NxResult;

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
    pub fn init(&self, buf: usize, size: usize) -> Result<(), NxResult> {
        todo!();

        if detect::version_above_400() && (!detect::version_above_500() || detect::has_kernel_patch()) {
            self.jtype = JitType::JitMemory;
        } else {
            /* TODO: implement env */
            self.jtype = JitType::CodeMemory;
        }

        let aligned_size = (size + (PAGE_SIZE - 1)) & !(PAGE_SIZE - 1);

        todo!();
    }
    
    pub fn to_writable() -> Result<(), NxResult> {
        todo!();
    }

    pub fn to_executable() -> Result<(), NxResult> {
        todo!();
    }
}