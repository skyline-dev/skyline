#![feature(proc_macro_hygiene)]

skyline::set_module_name!("skyline");

fn main() {
    println!("Test");
}

#[no_mangle]
extern "C" fn __custom_init() {
    main();
}

#[no_mangle]
extern "C" fn __custom_fini() {}
