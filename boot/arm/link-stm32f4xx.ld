/* STM32F401xC: 256KB Flash, 64KB SRAM */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
  RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 64K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
ENTRY(_start)

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  .text : ALIGN(4) {
    *(.text .text.*)
    *(.literal .literal.*)
    *(.glue_7)
    *(.glue_7t)
    . = ALIGN(4);
    _etext = .;
  } > FLASH

  .rodata : ALIGN(4) {
    *(.rodata .rodata.*)
    . = ALIGN(4);
  } > FLASH

  _sidata = LOADADDR(.data);

  .data : ALIGN(4) {
    _sdata = .;
    *(.data .data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT> FLASH

  .bss : ALIGN(4) {
    _sbss = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM
}
