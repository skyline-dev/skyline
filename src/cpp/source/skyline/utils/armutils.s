.macro CODE_BEGIN name
	.section .text.\name, "ax", %progbits
	.global \name
	.type \name, %function
	.align 2
	.cfi_startproc
\name:
.endm

.macro CODE_END
	.cfi_endproc
.endm

.macro armBackupRegisters
    sub sp, sp, #0x100
    stp x29, x30, [sp, #0xE8]
    stp x27, x28, [sp, #0xD8]
    stp x25, x26, [sp, #0xC8]
    stp x23, x24, [sp, #0xB8]
    stp x21, x22, [sp, #0xA8]
    stp x19, x20, [sp, #0x98]
    stp x17, x18, [sp, #0x88]
    stp x15, x16, [sp, #0x78]
    stp x13, x14, [sp, #0x68]
    stp x11, x12, [sp, #0x58]
    stp x9, x10, [sp, #0x48]
    stp x7, x8, [sp, #0x38]
    stp x5, x6, [sp, #0x28]
    stp x3, x4, [sp, #0x18]
    stp x1, x2, [sp, #0x8]
    str x0, [sp, #0x0]
.endm

.macro armRecoverRegisters
    ldr x0, [sp, #0x0]
    ldp x1, x2, [sp, #0x8]
    ldp x3, x4, [sp, #0x18]
    ldp x5, x6, [sp, #0x28]
    ldp x7, x8, [sp, #0x38]
    ldp x9, x10, [sp, #0x48]
    ldp x11, x12, [sp, #0x58]
    ldp x13, x14, [sp, #0x68]
    ldp x15, x16, [sp, #0x78]
    ldp x17, x18, [sp, #0x88]
    ldp x19, x20, [sp, #0x98]
    ldp x21, x22, [sp, #0xA8]
    ldp x23, x24, [sp, #0xB8]
    ldp x25, x26, [sp, #0xC8]
    ldp x27, x28, [sp, #0xD8]
    ldp x29, x30, [sp, #0xE8]
    add sp, sp, #0x100
.endm


CODE_BEGIN inlineHandlerImpl
    armBackupRegisters

    // branch and link to hook callback
    mov x0, sp
    ldr x16, [x17, #8]
    blr x16

    armRecoverRegisters

    // branch to trampoline
    ldr x16, [x17, #0x10]
    br x16

CODE_END


.global inlineHandlerStart
.global inlineHandlerEnd

CODE_BEGIN inlineHandler
inlineHandlerStart:
    adr x17, inlineHandlerEnd
    ldr x16, [x17]
    br x16

CODE_END
inlineHandlerEnd:
