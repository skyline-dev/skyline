use std::path::Path;

fn main() {
    // TODO: link in old skyline stuff
    // TODO: Get it to work with MSVC too?
    let headers_path = Path::new("src/cpp/include");
    let efl_headers_path = Path::new("src/cpp/libs/libeiffel/include");

    cc::Build::new()
        //.cpp(true)
        .warnings(false)
        .flag("-fno-rtti")
        .flag("-fomit-frame-pointer")
        .flag("-fno-exceptions")
        .flag("-fno-asynchronous-unwind-tables")
        .flag("-fno-unwind-tables")
        .flag("-enable-libstdcxx-allocator=new")
        .flag("-fpermissive")
        .file("src/cpp/source/main.cpp")
        .include(headers_path)
        .include(efl_headers_path)
        //.cpp_link_stdlib("stdc++")
        .compile("skyline");
        //println!("cargo:rerun-if-changed=src/cpp/source/main.cpp");
        println!("cargo:rerun-if-changed=build.rs");
}
