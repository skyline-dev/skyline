use std::ffi::CString;

use crate::Logger;

pub struct KernelLogger;

impl Logger for KernelLogger {
    fn log(&self, data: &[u8]) {
        // Probably want to check for the null-terminator manually instead of doing this and append it if it is missing
        unsafe {
            let bytes = CString::new(data).unwrap().as_bytes_with_nul();

            // Probably requires svc impls
            // svcOutputDebugString(bytes.as_ptr(), bytes.len());
        }
    }
}