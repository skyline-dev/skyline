use std::ffi::CString;
use crate::nx::kern::*;

use crate::Logger;

pub struct KernelLogger;

impl Logger for KernelLogger {
    fn log(&self, data: &[u8]) {
        svc::output_debug_string(data, data.len()).unwrap();
    }
}