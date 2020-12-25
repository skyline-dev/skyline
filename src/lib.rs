#![feature(proc_macro_hygiene)]

mod init;
mod pointer_iter;

extern "C" {
    fn skyline_init();
}

skyline::set_module_name!("skyline");

fn main() {
    println!("Trying to call skyline_init");
    unsafe { skyline_init(); }
}
