use std::fs;

use std::path::{
    Path,
    PathBuf
};

fn get_cpp_files<P: AsRef<Path>>(dir: P) -> Vec<PathBuf> {
    fs::read_dir(dir).unwrap().flat_map(|entry| {
        let entry = entry.unwrap();
        let path = entry.path();

        if entry.file_type().unwrap().is_file() {
            match path.extension().unwrap().to_string_lossy().into_owned().as_ref() {
                "c" | "cpp" | "s" => Vec::from([path]).into_iter(),
                _ => Vec::from([]).into_iter(),
            }
        } else {
            get_cpp_files(path).into_iter()
        }
    }).collect()
}

/// Requires WSL and Devkitpro for now
fn main() {
    // TODO: link in old skyline stuff
    // TODO: Get it to work with MSVC too?
    let headers_path = Path::new("src/cpp/include");
    let efl_headers_path = Path::new("src/cpp/libs/libeiffel/include");
    let dkp_headers_path = Path::new("/opt/devkitpro/libnx/include");

    println!("cargo:rerun-if-changed=build.rs");
    let source_files = get_cpp_files("src/cpp/source/");
    for file in &source_files {
        println!("cargo:rerun-if-changed={}", file.display());
    }
    // Needed to find libstdc++
    println!("cargo:rustc-link-search=/opt/devkitpro/devkitA64/aarch64-none-elf/lib");
    println!("cargo:rustc-link-search=/opt/devkitpro/devkitA64/lib/gcc/aarch64-none-elf/10.2.0");
    // Linker flags
    println!("cargo:rustc-cdylib-link-args=--shared --export-dynamic -nodefaultlibs");

    cc::Build::new()
        .compiler("/opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++")
        .cpp(true)
        //.cpp_link_stdlib("stdc++")
        .cpp_link_stdlib("gcc")
        .no_default_flags(true)
        .warnings(false)
        // CFLAGS
        .flag("-fPIC")
        .flag("-g")
        .flag("-Wall")
        .flag("-ffunction-sections")
        //.flag("-D__SWITCH__")
        // CXXFLAGS
        .flag("-fno-rtti")
        .flag("-fomit-frame-pointer")
        .flag("-fno-exceptions")
        .flag("-fno-asynchronous-unwind-tables")
        .flag("-fno-unwind-tables")
        .flag("-enable-libstdcxx-allocator=new")
        .flag("-fpermissive")
        // LIBS
        //.flag("-lgcc")
        //.flag("-lstdc++")
        .flag("-u malloc")
        .flag("-lc")
        // CPP
        .files(source_files)
        // HEADERS
        .include(headers_path)
        //.include(Path::new("src/cpp/include/nn/"))
        .include(Path::new("src/cpp/include/skyline/utils/"))
        .include(efl_headers_path)
        .include(dkp_headers_path)
        // internal.h
        .include("src/cpp/source/skyline/nx/")
        .compile("skyline");
}
