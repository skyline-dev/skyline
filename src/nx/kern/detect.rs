use crate::nx::kern::svc;
use crate::nx::result::kern;
use crate::nx::kern::types::*;
use std::alloc::{alloc, Layout};

lazy_static! {
    static ref KERNEL_VERSION       : usize  = detect_kernel_version();
    static ref KERNEL_PATCH_PRESENT : bool   = detect_kernel_patch();
}

fn detect_kernel_version() -> usize {
    let check_arr = [
        InfoType::AliasRegionAddress, 
        InfoType::ProgramId, 
        InfoType::InitialProcessIdRange, 
        InfoType::UserExceptionContextAddress, 
        InfoType::TotalNonSystemMemorySize
    ];

    check_arr.iter()
        /* Test if the InfoType is valid for kernel */
        .map(|&i| svc::get_info(i, Handle::INVALID, 0).err()) 
        /* Find the position of the first InfoType not recognized by the kernel */
        .position(|opt| opt.contains(&kern::INVALID_ENUM_VALUE)) 
        /* If all succeeded, we are the latest version detected. */
        .unwrap_or(check_arr.len()) 
        /* One-based index */
        + 1 
}

fn detect_kernel_patch() -> bool {
    unsafe {
        /* Allocate a page to initialize for CodeMemory. */
        let layout = Layout::from_size_align(PAGE_SIZE, PAGE_SIZE).unwrap();
        let heap = alloc(layout);
        
        /* Create a code memory for testing. */
        let code = svc::create_code_memory(heap as usize, PAGE_SIZE).unwrap();
        /* Test a code memory operation. */
        let test = svc::control_code_memory(code, CodeMapOperation::Invalid, 0, PAGE_SIZE, MemoryPermission::NONE);
        /* Clean up the code memory handle. */
        svc::close_handle(code).unwrap();
        /* Clean up page allocated. */
        std::alloc::dealloc(heap, layout);
        /* This result will only be returned if there's a kernel patch allowing JIT. */
        test.contains_err(&kern::INVALID_ENUM_VALUE)
    }
}

/* TODO */
/* 
macro_rules! create_ver_check {
    ($ver:expr) => {
        pub fn concat_idents!(version_above_, $ver)00() -> bool {
            KERNEL_VERSION > $ver
        }
    };
}

create_ver_check!(4);

*/

pub fn version_above_200() -> bool {
    KERNEL_VERSION.ge(&2)
}

pub fn version_above_300() -> bool {
    KERNEL_VERSION.ge(&3)
}

pub fn version_above_400() -> bool {
    KERNEL_VERSION.ge(&4)
}

pub fn version_above_500() -> bool {
    KERNEL_VERSION.ge(&5)
}

pub fn has_kernel_patch() -> bool {
    KERNEL_PATCH_PRESENT.eq(&true)
}