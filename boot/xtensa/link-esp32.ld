MEMORY
{
  iram (RWX) :  org = 0x40080000, len = 0xfc00 /* 32K */
  dram (RW) :   org = 0x3FFF0000, len = 0x10000 /** 64K*/
}

STACK_SIZE = 0x2000;

ENTRY(_start)
SECTIONS
{
  .text.vectors : ALIGN(0x1000) {
    __vector_base = .;
    KEEP(*(.window_overflow_4.text))
    . = ALIGN(64);
    KEEP(*(.window_underflow_4.text))
    . = ALIGN(64);
    KEEP(*(.window_overflow_8.text))
    . = ALIGN(64);
    KEEP(*(.window_underflow_8.text))
    . = ALIGN(64);
    KEEP(*(.WindowOverflow12.text))
    . = ALIGN(64);
    KEEP(*(.WindowUnderflow12.text))
    . = ALIGN(64);
    KEEP(*(.Level2InterruptVector.text))
    . = ALIGN(64);
    KEEP(*(.Level3InterruptVector.text))
    . = ALIGN(64);
    KEEP(*(.Level4InterruptVector.text))
    . = ALIGN(64);
    KEEP(*(.Level5InterruptVector.text))
    . = ALIGN(64);
    KEEP(*(.DebugExceptionVector.text))
    . = ALIGN(64);
    KEEP(*(.NMIExceptionVector.text))
    . = ALIGN(64);
    KEEP(*(.kernel_exception.text))
    . = ALIGN(64);
    KEEP(*(.user_exception.text))
    . = ALIGN(128);
    KEEP(*(.DoubleExceptionVector.text))
    . = ALIGN(64);
    . = ALIGN(0x400);
    KEEP(*(.vector))
    KEEP(*(.vector*))
    KEEP(*(.irq*))
  } > iram

  .text : ALIGN(16) {
    __start = .;
    KEEP(*(.text.boot))
    *(.literal .literal.*)
    *(.text .text.*)
  } > iram

  .bss (NOLOAD) : ALIGN(4) {
    _bss_start = ABSOLUTE(.);
    *(.bss .bss.*)
    *(COMMON)
    _bss_end = ABSOLUTE(.);
  } > dram

  .data : ALIGN(4) {
    _data_start = ABSOLUTE(.);
    *(.data .data*)
    *(.rodata .rodata.*)
    _data_end = ABSOLUTE(.);
  } > dram

  .stack (NOLOAD) : ALIGN(16) {
    _sstack = .;
    . = . + STACK_SIZE;
    _estack = .;
  } > dram

  sp_top = _estack;
}
