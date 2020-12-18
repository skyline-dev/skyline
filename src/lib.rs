#![feature(proc_macro_hygiene)]

skyline::set_module_name!("skyline");

fn main() {
    use std::io::Write;
    let mut test = std::net::TcpStream::connect(("192.168.0.1", 6969)).unwrap();

    test.write(b"test\n").unwrap();
}

#[no_mangle]
extern "C" fn __custom_init() {
    main();
}

#[no_mangle]
extern "C" fn __custom_fini() {}
