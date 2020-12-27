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

fn get_header_files<P: AsRef<Path>>(dir: P) -> Vec<PathBuf> {
    fs::read_dir(dir).unwrap().flat_map(|entry| {
        let entry = entry.unwrap();
        let path = entry.path();

        if entry.file_type().unwrap().is_file() {
            match path.extension().unwrap().to_string_lossy().into_owned().as_ref() {
                "h" => Vec::from([path]).into_iter(),
                _ => Vec::from([]).into_iter(),
            }
        } else {
            get_cpp_files(path).into_iter()
        }
    }).collect()
}

fn find_libgcc_folder() -> impl std::fmt::Display {
    fs::read_dir("/opt/devkitpro/devkitA64/lib/gcc/aarch64-none-elf")
        .unwrap()
        .filter_map(|entry| {
            let entry = entry.unwrap();
            if entry.file_type().unwrap().is_dir() {
                let path = entry.path().join("pic");
                if path.exists() {
                    Some(path.display().to_string())
                } else {
                    None
                }
            } else {
                None
            }
        })
        .next()
        .expect("No libgcc folder found. is devkitA64 installed?")
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
    for file in get_cpp_files("src/cpp/include/") {
        println!("cargo:rerun-if-changed={}", file.display());
    }
    // Needed to find stdc++ and gcc
    println!("cargo:rustc-link-search=/opt/devkitpro/devkitA64/aarch64-none-elf/lib/pic");
    println!("cargo:rustc-link-search={}", find_libgcc_folder());
    // Static libraries
    println!("cargo:rustc-link-lib=static=stdc++");
    println!("cargo:rustc-link-lib=static=gcc");

    // Linker flags
    println!("cargo:rustc-cdylib-link-args=--shared --export-dynamic -nodefaultlibs");

    cc::Build::new()
        .compiler("/opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++")
        .cpp(true)
        .shared_flag(true)
        .static_flag(true)
        .no_default_flags(true)
        .warnings(false)

        // DEFINES
        .define("__SWITCH__", None)

        // CFLAGS
        .flag("-fPIC")
        .flag("-g")
        .flag("-Wall")
        .flag("-ffunction-sections")
        //.flag("-v")

        // CXXFLAGS
        .flag("-fno-rtti")
        .flag("-fomit-frame-pointer")
        .flag("-fno-exceptions")
        .flag("-fno-asynchronous-unwind-tables")
        .flag("-fno-unwind-tables")
        .flag("-enable-libstdcxx-allocator=new")
        .flag("-fpermissive")
        
        // LIBS
        //.flag("-u malloc")

        // CPP
        .files(source_files)

        // HEADERS
        .include(headers_path)
        .include(efl_headers_path)
        .include(dkp_headers_path)
        // internal.h
        .include("src/cpp/source/skyline/nx/")
        .compile("skyline");
}
