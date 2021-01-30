macro_rules! reg {
    ( $func_name:ident, $register_name:ident ) => {
            pub fn $func_name() -> u64 {
                let ret: u64;

                unsafe {
                    asm!(
                        concat!("mrs x0, ", stringify!($register_name))
                        : "={x0}"(ret)
                    )
                }

                ret
            }
    };
}


reg!(get_system_tick,       cntpct_el0);
reg!(get_system_tick_freq,  cntfrq_el0);
reg!(get_cache_type,        ctr_el0);
reg!(get_tls,               tpidrro_el0);