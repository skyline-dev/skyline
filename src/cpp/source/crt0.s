.section ".text.crt0","ax"
.global __module_start
.extern __nx_module_runtime

__module_start:
    b startup
    .word __nx_mod0 - __module_start

.org __module_start+0x80
startup:
    // save lr
    mov  x7, x30

    // get aslr base
    bl   +4
    sub  x6, x30, #0x88

    // context ptr and main thread handle
    mov  x5, x0
    mov  x4, x1
bssclr_start:
    mov x27, x7
    mov x25, x5
    mov x26, x4

    // clear .bss
    adrp x0, __bss_start__
    adrp x1, __bss_end__
    add  x0, x0, #:lo12:__bss_start__
    add  x1, x1, #:lo12:__bss_end__
    sub  x1, x1, x0  // calculate size
    add  x1, x1, #7  // round up to 8
    bic  x1, x1, #7

bss_loop:
    str  xzr, [x0], #8
    subs x1, x1, #8
    bne  bss_loop

    // store stack pointer
    mov  x1, sp
    adrp x0, __stack_top
    str  x1, [x0, #:lo12:__stack_top]

    // initialize system
    mov  x0, x25
    mov  x1, x26
    mov  x2, x27
    .word deadbeef

.section ".rodata.mod0"
.global __nx_mod0
__nx_mod0:
    .ascii "MOD0"
    .word  __dynamic_start__    - __nx_mod0
    .word  __bss_start__        - __nx_mod0
    .word  __bss_end__          - __nx_mod0
    .word  0
    .word  0
    .word  __nx_module_runtime  - __nx_mod0
