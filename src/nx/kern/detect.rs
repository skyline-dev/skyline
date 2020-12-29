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
        .map(|&i| svc::get_info(i, Handle::INVALID, 0).err()) /* Test if the InfoType is valid for kernel */
        .position(|opt| opt.contains(&kern::INVALID_ENUM_VALUE)) /* Find the position of the first InfoType not recognized by the kernel */
        .unwrap_or(check_arr.len()) /* If all succeeded, we are the latest version detected. */
        + 1 /* One-based index */
}

fn detect_kernel_patch() -> bool {
    unsafe {
        /* Allocate a page to initialize for CodeMemory */
        let heap = alloc(Layout::from_size_align(PAGE_SIZE, PAGE_SIZE).unwrap());
        /* Create a code memory for testing */
        let code = svc::create_code_memory(heap as usize, PAGE_SIZE).unwrap();
        /* Test a code memory operation */
        let test = svc::control_code_memory(code, CodeMapOperation::Invalid, 0, PAGE_SIZE, MemoryPermission::NONE);
        /* Clean up the code memory handle */
        svc::close_handle(code).unwrap();
        /* This result will only be returned if there's a kernel patch allowing JIT */
        test.contains_err(&kern::INVALID_ENUM_VALUE)
    }
}

/* TODO */
/*macro_rules! create_ver_check {
    ($ver:ident) => {
        pub fn kernel_above_($ver)00() -> bool {
            KERNEL_VERSION > $ver
        }
    };
}

create_ver_check!(400); */

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