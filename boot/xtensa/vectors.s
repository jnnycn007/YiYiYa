.global __boot_double_exception
.extern l2_handler
.extern l3_handler
.extern l4_handler
.extern l5_handler
.extern debug_excetpion_handler
.extern nmi_excetpion_handler
.extern kernel_excetpion_handler
.extern user_excetpion_handler
.extern double_excetpion_handler

.section ".window_overflow_4.text", "ax"
.align 4
window_overflow_4:
    s32e    a0, a5, -16
    s32e    a1, a5, -12
    s32e    a2, a5, -8
    s32e    a3, a5, -4
    rfwo

.section ".window_underflow_4.text", "ax"
.align 4
window_underflow_4:
    l32e    a0, a5, -16
    l32e    a1, a5, -12
    l32e    a2, a5, -8
    l32e    a3, a5, -4
    rfwu

.section ".window_overflow_8.text", "ax"
.align 4
window_overflow_8:
    s32e    a0, a9, -16
    l32e    a0, a1, -12
    s32e    a1, a9, -12
    s32e    a2, a9, -8
    s32e    a3, a9, -4
    s32e    a4, a9, -32
    s32e    a5, a9, -28
    s32e    a6, a9, -24
    s32e    a7, a9, -20
    rfwo

.section ".window_underflow_8.text", "ax"
.align 4
window_underflow_8:
    l32e    a0, a9, -16
    l32e    a8, a9, -12
    l32e    a2, a9, -8
    l32e    a7, a8, -12
    l32e    a3, a9, -4
    l32e    a4, a7, -32
    l32e    a5, a7, -28
    l32e    a6, a7, -24
    l32e    a7, a7, -20
    rfwu

.section ".WindowOverflow12.text", "ax"
.align 4
WindowOverflow12:
    s32e    a0, a13, -16
    l32e    a0, a1, -12
    s32e    a1, a13, -12
    s32e    a2, a13, -8
    s32e    a3, a13, -4
    s32e    a4, a13, -48
    s32e    a5, a13, -44
    s32e    a6, a13, -40
    s32e    a7, a13, -36
    s32e    a8, a13, -32
    s32e    a9, a13, -28
    s32e    a10, a13, -24
    s32e    a11, a13, -20
    rfwo

.section ".WindowUnderflow12.text", "ax"
.align 4
WindowUnderflow12:
    l32e    a0, a13, -16
    l32e    a1, a13, -12
    l32e    a2, a13, -8
    l32e    a11, a1, -12
    l32e    a3, a13, -4
    l32e    a4, a11, -48
    l32e    a5, a11, -44
    l32e    a6, a11, -40
    l32e    a7, a11, -36
    l32e    a8, a11, -32
    l32e    a9, a11, -28
    l32e    a10, a11, -24
    l32e    a11, a11, -20
    rfwu

.section ".Level2InterruptVector.text", "ax"
.align 4
Level2InterruptVector:
    j l2_handler

.section ".Level3InterruptVector.text", "ax"
.align 4
Level3InterruptVector:
    j l3_handler

.section ".Level4InterruptVector.text", "ax"
.align 4
Level4InterruptVector:
    j l4_handler

.section ".Level5InterruptVector.text", "ax"
.align 4
Level5InterruptVector:
    j l5_handler

.section ".DebugExceptionVector.text", "ax"
.align 4
DebugExceptionVector:
    j debug_excetpion_handler

.section ".NMIExceptionVector.text", "ax"
.align 4
NMIExceptionVector:
    j nmi_excetpion_handler

.section ".kernel_exception.text", "ax"
.align 4
kernel_exception:
    j kernel_excetpion_handler

.section ".user_exception.text", "ax"
.align 4
user_exception:
    j user_excetpion_handler

.section ".DoubleExceptionVector.text", "ax"
.align 4
DoubleExceptionVector:
    j double_excetpion_handler
__boot_double_exception:
    j __boot_double_exception
