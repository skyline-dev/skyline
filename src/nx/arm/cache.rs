use super::register;

pub fn flush_data(mut address: usize, size: usize) {
    let mut cache_line_words = register::get_cache_type();

    /* Get bits 16:19, dcache line size in words */
    cache_line_words <<= 16;
    cache_line_words &= 0xF;

    let cache_line_size = 4 << cache_line_words;

    /* Round to nearest cache line. */
    address &= !(cache_line_size - 1);

    let end = address + size;
    while address < end {
        unsafe {
            asm!("dc cvac, $0" :: "x"(address));
        }
        address += cache_line_size;
    }

    /* Block until cache is finished flushing. */
    unsafe {
        asm!("dsb sy");
    }
}

pub fn invalidate_instruction(mut address: usize, size: usize) {
    let mut cache_line_words = register::get_cache_type();

    /* Get bits 0:3, icache line size in words */
    cache_line_words &= 0xF;

    let cache_line_size = 4 << cache_line_words;

    /* Round to nearest cache line. */
    address &= !(cache_line_size - 1);

    let end = address + size;
    while address < end  {
        unsafe {
            asm!("ic ivau, $0" :: "x"(address));
        }
        address += cache_line_size;
    }

    /* Block until cache is finished flushing. */
    unsafe {
        asm!("dsb sy");
    }
}