use std::str;
use crate::nx::kern::*;

use crate::Logger;

pub struct KernelLogger;

impl Logger for KernelLogger {
    fn log(&self, data: &[u8]) {
        let string = match str::from_utf8(data.into()) {
            Ok(str) => str,
            Err(err) => panic!("Couldn't convert KernelLogger message to a UTF8 string: {}", err),
        };
        
        svc::output_debug_string(string).unwrap();
    }
}