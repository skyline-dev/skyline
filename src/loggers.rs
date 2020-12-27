mod sd_logger;
mod kernel_logger;

pub use sd_logger::*;
pub use kernel_logger::*;

pub trait Logger {
    fn log(&self, data: &[u8]);
}