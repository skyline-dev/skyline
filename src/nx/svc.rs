
use crate::nx::result;
use crate::nx::result::NxResult;

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
    let res : NxResult;
    let out: usize;

    unsafe {
        asm!(
            "svc 0x0"
            :  "={W0}" (res), "={X1}"(out)
            :  "X1" (size) // TODO why can't this be W1?
        );
    }

    check_res!(res, out);
}

pub fn output_debug_string(string: &str) -> Result<(), NxResult> {
    let res : NxResult;

    let ptr = string.as_ptr();
    let size = string.len();

    unsafe {
        asm!(
            "svc 0x27"
            :  "={W0}" (res)
            :  "X0" (ptr), "W1" (size)
        );
    }

    check_res!(res, ());
}