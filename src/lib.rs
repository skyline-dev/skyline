#![feature(proc_macro_hygiene)]

mod api;
mod init;
mod pointer_iter;
//mod loggers;

//pub use loggers::{Logger, SdLogger, KernelLogger};

extern "C" {
    fn skyline_init();
}

skyline::set_module_name!("skyline");

fn main() {
    println!("Trying to call skyline_init");
    //let logger: Box<dyn Logger> = Box::new(SdLogger);
    //logger.log(b"test");
    unsafe { skyline_init(); }
}
