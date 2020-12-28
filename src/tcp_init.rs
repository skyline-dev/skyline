use skyline::nn;
use skyline::libc;
use std::alloc::{alloc, Layout};
use std::sync::atomic::{AtomicBool, Ordering};

const POOL_ALIGN: usize = 0x4000;
const POOL_SIZE: usize = 0x600000;
const ALLOC_POOL_SIZE: usize = 0x20000;
const CONCURENCY_LIMIT: usize = 14;

static IS_INIT: AtomicBool = AtomicBool::new(false);

pub fn init_tcp() {
    if IS_INIT.swap(true, Ordering::SeqCst) {
        return
    }

    unsafe {
        let pool = alloc(Layout::from_size_align(POOL_SIZE, POOL_ALIGN).unwrap());

        nn::socket::Initialize(
            pool as *mut libc::c_void,
            POOL_SIZE as _,
            ALLOC_POOL_SIZE as _,
            CONCURENCY_LIMIT as _,
        );
    }

    #[skyline::hook(replace = nn::socket::Initialize)]
    fn stubbed_tcp_init() -> u32 {
        0
    }

    #[skyline::hook(replace = nn::socket::Finalize)]
    fn stubbed_tcp_deinit() -> u32 {
        0
    }

    skyline::install_hooks!(stubbed_tcp_init, stubbed_tcp_deinit);
}
