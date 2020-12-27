#![feature(proc_macro_hygiene)]
mod nx;
mod api;
mod init;
mod pointer_iter;
//mod loggers;

//pub use loggers::{Logger, SdLogger, KernelLogger};

extern "C" {
    fn skyline_main();
    fn virtmemSetup();
    fn utils_init();
}

skyline::set_module_name!("skyline");

fn main() {
    println!("Trying to call skyline_main");
    //let logger: Box<dyn Logger> = Box::new(SdLogger);
    //logger.log(b"test");

    unsafe {
        utils_init();
        virtmemSetup();  // needed for libnx JIT

        skyline_main();
    }
}
