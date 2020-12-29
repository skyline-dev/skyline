#![feature(proc_macro_hygiene)]
#![feature(new_uninit)]
#![feature(asm)]
#![feature(option_result_contains)]
#![feature(result_contains_err)]

#[macro_use]
extern crate bitflags;
#[macro_use]
extern crate lazy_static;

mod nx;
mod api;
mod init;
mod pointer_iter;
mod tcp_init;
//mod loggers;

//pub use loggers::{Logger, SdLogger, KernelLogger};

extern "C" {
    fn skyline_main();
    fn virtmemSetup();
    fn utils_init();
    fn populate_process_handle();
    fn A64HookInit();
}

skyline::set_module_name!("skyline");

fn initialize_process() {
    unsafe {
        utils_init();
        virtmemSetup();  // needed for libnx JIT

        // populate our own process handle
        populate_process_handle();

        // init hooking setup
        A64HookInit();
    }
}

fn main() {
    println!("[skyline] entered `main`");

    initialize_process();
    tcp_init::init_tcp();

    println!("[skyline] Process initialization complete");

    unsafe {
        skyline_main();
    }
}
