use crate::{pointer_iter::StartEndArrayIterator};

type PreInitFn = extern "C" fn();
type InitFn = extern "C" fn();
type FiniFn = extern "C" fn();

extern "C" {
    static __preinit_array_start__: PreInitFn;
    static __preinit_array_end__: PreInitFn;

    static __init_array_start__: InitFn;
    static __init_array_end__: InitFn;

    static __fini_array_start__: FiniFn;
    static __fini_array_end__: FiniFn;
}

use crate::nx::kern::svc;
use crate::nx::util::cur_proc_handle;

#[no_mangle]
unsafe extern "C" fn __custom_init() {
    for preinit_func in StartEndArrayIterator::new(&__preinit_array_start__, &__preinit_array_end__) {
        preinit_func();
    }
    
    for init_func in StartEndArrayIterator::new(&__init_array_start__, &__init_array_end__) {
        init_func();
    }

    svc::output_debug_string("Hello Rust!\n").unwrap();

    let proc: u32 = cur_proc_handle::get().into();
    
    svc::output_debug_string("Current process handle: ").unwrap();
    svc::output_debug_string(&proc.to_string()).unwrap();

    svc::break_();

    crate::main();
}

#[no_mangle]
unsafe extern "C" fn __custom_fini() {
    for fini_func in StartEndArrayIterator::new(&__fini_array_start__, &__fini_array_end__) {
        fini_func();
    }
}
