use skyline::hooks::Region;

// generate a shim for the exported API for functions that are from C/C++
macro_rules! extern_forward {
    (
        $(
            #[link_name = $link_name:ident]
            fn $name:ident ( $($arg:ident : $ty:ty ),* $(,)? ) $(-> $ret:ty)?;
        )+
    ) => {
        $(
            extern "C" {
                fn $link_name( $($arg : $ty ),*) $(-> $ret)?;
            }

            #[no_mangle]
            unsafe fn $name ( $($arg : $ty ),* ) $(-> $ret)? {
                $link_name( $( $arg ),*)
            }
        )+
    };
}

extern_forward! {
    #[link_name = skyline_tcp_send_raw_impl]
    fn skyline_tcp_send_raw(data: *const u8, size: u64);

    #[link_name = getRegionAddress_impl]
    fn getRegionAddress(region: Region) -> *const ();

    #[link_name = A64HookFunction_impl]
    fn A64HookFunction(symbol: *const (), replace: *const (), result: *mut *const ());

    #[link_name = A64InlineHook_impl]
    fn A64InlineHook(symbol: *const (), replace: *const ());

    #[link_name = sky_memcpy_impl]
    fn sky_memcpy(dest: *mut u8, src: *const u8, n: usize) -> u64;

    #[link_name = get_program_id_impl]
    fn get_program_id() -> u64;
}
