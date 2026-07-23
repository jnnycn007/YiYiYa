.section ".text.boot"
.global _start
.global boot_jump_to_kernel
.extern _estack
.align 16
_start:

// Use a minimal call0 entry path for early boot stability.
movi a1, _estack

call0 init_boot

halt:
    j halt

boot_jump_to_kernel:
    mov a6, a2
    mov a2, a3
    mov a3, a4
    mov a4, a5
    callx0 a6
    ret.n