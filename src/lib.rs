#![feature(proc_macro_hygiene)]

extern {
    fn skyline_init();
}

skyline::set_module_name!("skyline");

fn main() {
    println!("Trying to call skyline_init");
    unsafe { skyline_init(); }
}

#[no_mangle]
extern "C" fn __custom_init() {
    main();
}

#[no_mangle]
extern "C" fn __custom_fini() {}
