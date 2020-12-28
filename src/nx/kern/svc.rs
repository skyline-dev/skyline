use std::mem::MaybeUninit;
use crate::nx::result;
use crate::nx::result::NxResult;
use crate::nx::kern::types;

macro_rules! check_res {
    ($res:expr, $out:expr) => {
        if $res == result::SUCCESS {
            return Ok($out);
        }
        else {
            return Err($res);
        }
    };
}

pub fn set_heap_size(size: usize) -> Result<usize, NxResult> {
    let res: NxResult;
    let out: usize;

    unsafe {
        asm!(
            "
            mov x1, {3}
            svc 0x1
            "
            :  "={W0}" (res), "={X1}"(out)
            :  "r" (size)
        );
    }

    check_res!(res, out);
}

pub fn set_memory_permission(address: usize, size: usize, permission: types::MemoryPermission) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            mov x0, {1}
            mov x1, {2}
            mov w2, {3}
            svc 0x2
            "
            : "={W0}" (res)
            : "r" (address), "r" (size), "r" (permission)
        );
    }

    check_res!(res, ());
}

pub fn set_memory_attribute(address: usize, size: usize, mask: u32, value: u32) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            mov x0, {1}
            mov x1, {2}
            mov w2, {3}
            mov w3, {4}
            svc 0x3
            "
            : "={W0}" (res)
            : "r" (address), "r" (size), "r" (mask), "r" (value)
        );
    }

    check_res!(res, ());
}

pub fn map_memory(dst_address: usize, src_address: usize, size: usize) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            mov x0, {1}
            mov x1, {2}
            mov x2, {3}
            svc 0x4
            "
            : "={W0}" (res)
            : "r" (dst_address), "r" (src_address), "r" (size)
        );
    }

    check_res!(res, ());
}

pub fn unmap_memory(dst_address: usize, src_address: usize, size: usize) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            mov x0, {1}
            mov x1, {2}
            mov x2, {3}
            svc 0x5
            "
            : "={W0}" (res)
            : "r" (dst_address), "r" (src_address), "r" (size)
        );
    }

    check_res!(res, ());
}

pub struct QueryMemoryResult {
    meminfo: types::MemoryInfo,
    pageinfo: types::PageInfo,
}

pub fn query_memory(address: usize) -> Result<QueryMemoryResult, NxResult> {
    let res: NxResult;
    let meminfo = MaybeUninit::<types::MemoryInfo>::uninit();

    let svcres = unsafe {
        
        let meminfo_ptr = meminfo.as_ptr();
        let pageinfo: types::PageInfo;
        asm!(
            "
            mov x0, {2}
            mov x2, {3}
            svc 0x6
            "
            : "={W0}" (res), "={W1}" (pageinfo)
            : "r" (address), "r" (meminfo_ptr)
        );

        QueryMemoryResult {
            meminfo: meminfo.assume_init(), 
            pageinfo
        }
    };

    check_res!(res, svcres);
}

pub fn exit_process() -> ! {
    unsafe {
        asm!(
            "svc 0x7"
        );
    }

    unreachable!();
}

pub fn output_debug_string(string: &str) -> Result<(), NxResult> {
    let res: NxResult;

    let ptr = string.as_ptr() as usize;
    let size = string.len() as usize;

    unsafe {
        asm!(
            "
            mov x0, $1
            mov x1, $2
            svc 0x27
            "
            :  "={w0}"(res)
            :  "r"(ptr), "r"(size)
        );
    }

    check_res!(res, ());
}

pub fn get_info(info_type: types::InfoType, handle: types::Handle, sub_type: u64) -> Result<u64, NxResult> {
    let res: NxResult;
    let info: u64;

    unsafe {
        asm!(
            "
            mov w0, $2
            mov w1, $3
            mov x2, $4
            svc 0x27
            "
            :  "={w0}"(res), "={x1}"(info)
            :  "r"(info_type), "r"(handle), "r"(sub_type)
        );
    }

    check_res!(res, info);
}