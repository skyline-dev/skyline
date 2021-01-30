use std::mem::MaybeUninit;
use crate::nx::result;
use crate::nx::result::NxResult;
use crate::nx::kern::types::{*};

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
            svc 0x1
            "
            :  "={W0}" (res), "={X1}"(out)
            :  "{x1}" (size)
        );
    }

    check_res!(res, out);
}

pub fn set_memory_permission(address: usize, size: usize, permission: MemoryPermission) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x2
            "
            : "={W0}" (res)
            : "{x0}" (address), "{x1}" (size), "{w2}" (permission)
        );
    }

    check_res!(res, ());
}

pub fn set_memory_attribute(address: usize, size: usize, mask: u32, value: u32) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x3
            "
            : "={W0}" (res)
            : "{x0}" (address), "{x1}" (size), "{w2}" (mask), "{w3}" (value)
        );
    }

    check_res!(res, ());
}

pub fn map_memory(dst_address: usize, src_address: usize, size: usize) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x4
            "
            : "={W0}" (res)
            : "{x0}" (dst_address), "{x1}" (src_address), "{x2}" (size)
        );
    }

    check_res!(res, ());
}

pub fn unmap_memory(dst_address: usize, src_address: usize, size: usize) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x5
            "
            : "={W0}" (res)
            : "{x0}" (dst_address), "{x1}" (src_address), "{x2}" (size)
        );
    }

    check_res!(res, ());
}

pub struct QueryMemoryResult {
    pub meminfo: MemoryInfo,
    pub pageinfo: PageInfo,
}

pub fn query_memory(address: usize) -> Result<QueryMemoryResult, NxResult> {
    let res: NxResult;
    let meminfo = MaybeUninit::<MemoryInfo>::uninit();

    let svcres = unsafe {
        
        let meminfo_ptr = meminfo.as_ptr();
        let pageinfo: PageInfo;
        asm!(
            "
            svc 0x6
            "
            : "={W0}" (res), "={W1}" (pageinfo)
            : "{x0}" (address), "{x2}" (meminfo_ptr)
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

pub fn close_handle(handle: Handle) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x16
            "
            :   "={w0}"(res)
            :   "{w0}"(handle)
        );
    };

    check_res!(res, ());
}

pub fn wait_synchronization_single(handle: Handle, timeout: i32) -> Result<u32, NxResult> {
    wait_synchronization(&vec!(handle), timeout) 
}

pub fn wait_synchronization(handles: &Vec<Handle>, timeout: i32) -> Result<u32, NxResult> {
    let res: NxResult;
    let idx: u32;

    let handles_ptr = handles.as_ptr();
    let handles_count = handles.len();
    unsafe {
        asm!(
            "
            svc 0x18
            "
            :   "={w0}"(res), "={w1}"(idx)
            :   "{x1}"(handles_ptr), "{w2}"(handles_count), "{x3}"(timeout)
        );
    };

    check_res!(res, idx);
}

pub fn send_sync_request(handle: Handle) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x21
            "
            :   "={w0}"(res)
            :   "{w0}"(handle)
        );
    }

    check_res!(res, ());
}

pub fn break_() -> ! {
    unsafe {
        asm!(
            "svc 0x26"
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
            svc 0x27
            "
            :  "={w0}"(res)
            :  "{x0}"(ptr), "{x1}"(size)
        );
    }

    check_res!(res, ());
}

pub fn get_info(info_type: InfoType, handle: Handle, sub_type: u64) -> Result<u64, NxResult> {
    let res: NxResult;
    let info: u64;

    unsafe {
        asm!(
            "
            svc 0x29
            "
            :  "={w0}"(res), "={x1}"(info)
            :  "{w1}"(info_type), "{w2}"(handle), "{x3}"(sub_type)
        );
    }

    check_res!(res, info);
}

#[derive(Copy, Clone)]
pub struct CreateSessionResult {
    pub server : Handle,
    pub client : Handle,
}

pub fn create_session(is_light: bool, name: u64) -> Result<CreateSessionResult, NxResult> {
    let res: NxResult;

    let svcres = unsafe {
        
        let server: Handle;
        let client: Handle;
        asm!(
            "
            svc 0x40
            "
            : "={w0}" (res), "={w1}" (server), "={w2}" (client)
            : "{w2}" (is_light), "{x3}" (name)
        );

        CreateSessionResult {
            server,
            client
        }
    };

    check_res!(res, svcres);
}

pub fn reply_and_receive(handle_idx: &mut i32, handles: Vec<Handle>, reply_target_session_handle: Handle, timeout: u64) -> Result<(),NxResult> {
    let res: NxResult;

    /* Kernel may or may not initialize this value depending on result. We initialize as -1, as it would under some error scenarios. */
    #[allow(unused_assignments)]
    let mut idx = -1;
    
    let handles_ptr = handles.as_ptr();
    let handles_count = handles.len();
    unsafe {
        asm!(
            "
            svc 0x43
            "
            : "={w0}"(res), "={w1}"(idx)
            : "{x1}"(handles_ptr), "{w2}"(handles_count), "{w3}"(reply_target_session_handle), "{x4}"(timeout)
        );
    }

    *handle_idx = idx;
    check_res!(res, ());
}

pub fn create_code_memory(address: usize, size: usize) -> Result<Handle, NxResult> {
    let res: NxResult;
    let handle: Handle;

    unsafe {
        asm!(
            "
            svc 0x4b
            "
            :   "={w0}"(res), "={w1}"(handle)
            :   "{x1}"(address), "{x2}"(size)
        )
    };

    check_res!(res, handle);
}

pub fn control_code_memory(handle: Handle, op: CodeMapOperation, address: usize, size: usize, perm: MemoryPermission) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x4c
            "
            :   "={w0}"(res)
            :   "{w0}"(handle), "{w1}"(op), "{x2}"(address), "{x3}"(size), "{w4}"(perm)
        );
    };

    check_res!(res, ());
}

pub fn set_process_memory_permission(handle: Handle, address: usize, size: usize, perm: MemoryPermission) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x73
            "
            :   "={w0}"(res)
            :   "{w0}"(handle), "{x1}"(address), "{x2}"(size), "{w3}"(perm)
        );
    }

    check_res!(res, ());
}

pub fn map_process_code_memory(handle: Handle, dst_address: usize, src_address: usize, size: usize) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x77
            "
            :   "={w0}"(res)
            :   "{w0}"(handle), "{x1}"(dst_address), "{x2}"(src_address), "{x3}"(size)
        );
    }

    check_res!(res, ());
}

pub fn unmap_process_code_memory(handle: Handle, dst_address: usize, src_address: usize, size: usize) -> Result<(), NxResult> {
    let res: NxResult;

    unsafe {
        asm!(
            "
            svc 0x78
            "
            :   "={w0}"(res)
            :   "{w0}"(handle), "{x1}"(dst_address), "{x2}"(src_address), "{x3}"(size)
        )
    }

    check_res!(res, ());
}