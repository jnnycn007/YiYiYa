/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "init.h"
#ifdef SINGLE_KERNEL
#else
#include "esp32.h"
#endif
#include "gpio.h"
#include "soc/uart_reg.h"

static boot_info_t* boot_info = NULL;
static boot_info_t boot_data;
static char* kernel_envp[2];

typedef int (*entry)(int, char**, char**);
typedef void (*rom_write_char_uart_fn)(char c);
typedef u32 (*rom_spiflash_read_fn)(u32 src, u32* des, u32 len);
typedef unsigned int (*rom_cache_flash_mmu_set_fn)(int cpu_no, int pid,
                                                   unsigned int vaddr,
                                                   unsigned int paddr,
                                                   int psize, int num);
typedef void (*rom_cache_read_enable_fn)(int cpu_no);

typedef void (*rom_cache_flush_fn)(int cpu_no);
typedef void (*rom_cache_read_disable_fn)(int cpu_no);
typedef void (*kernel_entry_fn)(int argc, char** argv, char** envp);

volatile unsigned char* const UART0_PTR = (unsigned char*)0x0101f1000;

rom_spiflash_read_fn disk_read_lba = 0x40062ed8;
rom_write_char_uart_fn esp_send = 0x40007cf8;
rom_cache_flash_mmu_set_fn cache_flash_mmu_set = 0x400095e0;
rom_cache_read_enable_fn cache_read_enable = 0x40009a84;
rom_cache_flush_fn cache_flush = 0x40009a14;
rom_cache_read_disable_fn cache_read_disable = 0x40009ab8;

extern int _bss_start;
extern int _bss_end;
extern int _data_start;
extern int _data_end;
extern int _estack;

#define ESP32_DRAM_LOW  0x3FFC0000U
#define ESP32_DRAM_HIGH 0x40000000U
#define ESP32_HEAP_LOW  0x3FFD0000U
extern void boot_jump_to_kernel(kernel_entry_fn entry, int argc, char** argv,
                                char** envp);

void io_write32(uint port, u32 data) { *(u32*)port = data; }

u32 io_read32(uint port) {
  u32 data;
  data = *(u32*)port;
  return data;
}

void init_uart() {
  u32 addr;
  u32 val;
}

static inline void early_uart_send_ch(u8 c) {
  *(volatile u32*)UART_FIFO_REG(0) = c;
}

#define BOOT_PUTC(ch) (*(volatile u32*)UART_FIFO_REG(0) = (u8)(ch))
#define BOOT_NL()     \
  do {                \
    BOOT_PUTC('\r');  \
    BOOT_PUTC('\n');  \
  } while (0)

void uart_send_ch(u8 c) {
  if (c == '\n') {
    early_uart_send_ch('\r');
  }
  early_uart_send_ch(c);
}

char uart_getc() {
  // char r;
  // /* wait until something is in the buffer */
  // do{asm volatile("nop");}while(*UART0_FR&0x10);
  // /* read it and return */
  // r=(char)(*UART0_DR);
  // /* convert carrige return to newline */
  // return r=='\r'?'\n':r;
}

static void print_string(const unsigned char* str) {
  while (*str) {
    uart_send_ch(*str);
    ++str;
  }
}

void getch() { uart_getc(); }

static void print_char(char s) { uart_send_ch(s); }

void display(const char* string) { print_string(string); }

static void print_hex32(u32 value) {
  static const char hex[] = "0123456789abcdef";
  for (int shift = 28; shift >= 0; shift -= 4) {
    uart_send_ch(hex[(value >> shift) & 0xF]);
  }
}

static void print_label_hex(const char* label, u32 value) {
  print_string((const unsigned char*)label);
  print_hex32(value);
  uart_send_ch('\n');
}

static void print_triplet_hex(const char* label, u32 v1, u32 v2, u32 v3) {
  print_string((const unsigned char*)label);
  print_hex32(v1);
  uart_send_ch(' ');
  print_hex32(v2);
  uart_send_ch(' ');
  print_hex32(v3);
  uart_send_ch('\n');
}

static void print_quad_hex(const char* label, u32 v1, u32 v2, u32 v3, u32 v4) {
  print_string((const unsigned char*)label);
  print_hex32(v1);
  uart_send_ch(' ');
  print_hex32(v2);
  uart_send_ch(' ');
  print_hex32(v3);
  uart_send_ch(' ');
  print_hex32(v4);
  uart_send_ch('\n');
}

void init_boot_info() {
  boot_info = &boot_data;
  boot_info->version = BOOT_VERSION;
  boot_info->kernel_origin_base = KERNEL_ORIGIN_BASE;
  boot_info->kernel_base = KERNEL_BASE;
  boot_info->kernel_size = KERNEL_BLOCK_SIZE * READ_BLOCK_SIZE * 2;
  boot_info->tss_number = MAX_CPU;
}

void init_disk() {
  boot_info->disk.hpc = 2;
  boot_info->disk.spt = 18;
  boot_info->disk.type = 2;  // flash
}

void init_display() {
  boot_info->disply.mode = 1;
  boot_info->disply.video = 0xB8000;
  boot_info->disply.height = 25;
  boot_info->disply.width = 80;
}

void init_memory() {
  // read memory info
  int count = 0;
  memory_info_t* ptr = boot_info->memory;
  boot_info->total_memory = 0;

  u32 heap_start = (u32)&_estack;
  if (heap_start < ESP32_HEAP_LOW) {
    heap_start = ESP32_HEAP_LOW;
  }
  heap_start = (heap_start + 0xF) & ~0xFU;
  if (heap_start >= ESP32_DRAM_HIGH) {
    heap_start = ESP32_DRAM_HIGH;
  }

  ptr->base = (void*)heap_start;
  ptr->length = ESP32_DRAM_HIGH - heap_start;
  ptr->type = 1;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;

  boot_info->memory_number = count;
  // page setup
}
static inline void init_cpu() {
  // enable cr0
}

static inline void read_kernel() {
  // #ifdef KERNEL_MOVE
  //   u32 addr = boot_info->kernel_origin_base;
  // #else
  //   u32 addr = boot_info->kernel_base;
  // #endif

  //   u32 read_addr = KERNEL_FLASH_ADDR;
  //   printf("read kernel from flash %x to ram %x\n", read_addr, addr);
  //   u32 ret = disk_read_lba(read_addr, addr, KERNEL_SIZE);
  //   if (ret == 0) {
  //     printf("read kernel success\n");
  //   } else {
  //     printf("read kernel faild\n");
  //   }
}



void* memmove32(void* s1, const void* s2, u32 n) {
  u32 *dest, *src;
  int i;
  dest = (u32*)s1;
  src = (u32*)s2;
  for (i = 0; i < n / 4; i++) {
    dest[i] = src[i];
  }
}

void* boot_memset32(void* dest, int c, size_t n) {
  // Early boot only clears aligned sections. Keep this simple and ROM-free.
  int i;
  u32 word = (u32)c;
  u32* d = (u32*)dest;
  for (i = 0; i < n / 4; i++) {
    d[i] = word;
  }
  return dest;
}

void init_boot() {

#ifdef SINGLE_KERNEL

#else
  print_string((const unsigned char*)"bootloader init\n");
  bootloader_init();
  print_string((const unsigned char*)"bootloader init done\n");
#endif

  boot_memset32(&_bss_start, 0, (&_bss_end - &_bss_start) * sizeof(_bss_start));

  print_string((const unsigned char*)"bss cleared\n");
  init_uart();

  display("hello duck\n");
  init_boot_info();
  display("init boot info end\n");
  print_label_hex("boot info addr ", (u32)boot_info);

  print_string("init display\n");
  init_display();

  print_string("init memory\n");
  init_memory();

  print_string("init disk\n");
  init_disk();

  start_kernel();

  for (;;)
    ;
}

#define read16(addr) ((*(u32*)addr) & 0xff)

static void load_elf(Elf32_Ehdr* elf_header) {
  u32 e_phnum = read16(&elf_header->e_phnum);
  print_label_hex("e_phnum ", e_phnum);
  u32 elf = KERNEL_FLASH_ADDR;
  Elf32_Phdr phdr_data[PHDR_NUM];
  Elf32_Phdr* phdr = (elf + elf_header->e_phoff);
  disk_read_lba(phdr, &phdr_data, sizeof(Elf32_Phdr) * PHDR_NUM);
  phdr = &phdr_data;
  // printf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry = 0;
  for (int i = 0; i < e_phnum; i++) {
    print_label_hex("elf type ", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL:
        print_quad_hex("null seg ", phdr[i].p_offset, phdr[i].p_vaddr,
                       phdr[i].p_paddr, phdr[i].p_filesz);
        print_label_hex("null mem ", phdr[i].p_memsz);
        break;
      case PT_LOAD: {
        if ((phdr[i].p_flags & PF_X) == PF_X) {  // is code
          print_quad_hex("load x seg ", phdr[i].p_offset, phdr[i].p_vaddr,
                         phdr[i].p_paddr, phdr[i].p_filesz);
          print_triplet_hex("load x mem ", phdr[i].p_memsz, phdr[i].p_flags, 0);
          u32* start = elf + phdr[i].p_offset;
          u32* vaddr = phdr[i].p_vaddr;
          entry = vaddr;
          print_triplet_hex("code map ", (u32)start, (u32)vaddr, phdr[i].p_memsz);
          // u32 ret = disk_read_lba(start, vaddr, phdr[i].p_memsz);

          u32 irom_load_addr_aligned = (u32)vaddr & MMU_FLASH_MASK;
          u32 irom_page_count =
              (phdr[i].p_memsz +
               ((u32)vaddr - (((u32)vaddr) & MMU_FLASH_MASK)) + MMU_BLOCK_SIZE -
               1) /
              MMU_BLOCK_SIZE;
          int rc = cache_flash_mmu_set(0, 0, irom_load_addr_aligned,
                                       ((u32)start) & MMU_FLASH_MASK, 64,
                                       irom_page_count);

          rc |= cache_flash_mmu_set(1, 0, irom_load_addr_aligned,
                                    ((u32)start) & MMU_FLASH_MASK, 64,
                                    irom_page_count);

          print_quad_hex("code mmu ", irom_load_addr_aligned, (u32)start,
                         irom_page_count, rc);
        } else if ((phdr[i].p_flags & PF_R) == PF_R) {  // is data for write
          print_quad_hex("load r seg ", phdr[i].p_offset, phdr[i].p_vaddr,
                         phdr[i].p_paddr, phdr[i].p_filesz);
          print_triplet_hex("load r mem ", phdr[i].p_memsz, phdr[i].p_flags, 0);
          u32* start = elf + phdr[i].p_offset;
          u32* vaddr = phdr[i].p_vaddr;
          entry = vaddr;
          print_triplet_hex("rodata map ", (u32)start, (u32)vaddr,
                            phdr[i].p_memsz);
          // u32 ret = disk_read_lba(start, vaddr, phdr[i].p_memsz);
          u32 drom_load_addr_aligned = (u32)vaddr & MMU_FLASH_MASK;
          u32 drom_page_count =
              (phdr[i].p_memsz +
               ((u32)vaddr - (((u32)vaddr) & MMU_FLASH_MASK)) + MMU_BLOCK_SIZE -
               1) /
              MMU_BLOCK_SIZE;
          int rc = cache_flash_mmu_set(0, 0, drom_load_addr_aligned,
                                       ((u32)start) & MMU_FLASH_MASK, 64,
                                       drom_page_count);

          rc |= cache_flash_mmu_set(1, 0, drom_load_addr_aligned,
                                    ((u32)start) & MMU_FLASH_MASK, 64,
                                    drom_page_count);

          print_quad_hex("rodata mmu ", drom_load_addr_aligned, (u32)start,
                         drom_page_count, rc);
        } else {
          print_quad_hex("load other ", phdr[i].p_offset, phdr[i].p_vaddr,
                         phdr[i].p_paddr, phdr[i].p_filesz);
          print_triplet_hex("load flags ", phdr[i].p_memsz, phdr[i].p_flags, 0);
          u32* start = elf + phdr[i].p_offset;
          u32* vaddr = phdr[i].p_vaddr;
        }

      } break;
      default:
        break;
    }
  }

  Elf32_Shdr shdr_data[SHDR_NUM];
  Elf32_Shdr* shdr = (elf + elf_header->e_shoff);
  disk_read_lba(shdr, &shdr_data, sizeof(Elf32_Shdr) * SHDR_NUM);
  shdr = &shdr_data;

  u32 e_shnum = read16(&elf_header->e_shnum);
  print_label_hex("e_shnum ", e_shnum);
  for (int i = 0; i < e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      u32* vaddr = shdr[i].sh_addr;
      u32* phstart = (u32)elf + shdr[i].sh_offset;

      // u32 drom_load_addr_aligned = (u32)vaddr & MMU_FLASH_MASK;
      // u32 drom_page_count=(shdr[i].sh_size + ((u32)vaddr - (((u32)vaddr) &
      // MMU_FLASH_MASK)) + MMU_BLOCK_SIZE - 1) / MMU_BLOCK_SIZE; int rc =
      // cache_flash_mmu_set(0, 0,drom_load_addr_aligned , ((u32)phstart) &
      // MMU_FLASH_MASK, 64, drom_page_count);

      // rc |= cache_flash_mmu_set(1, 0, drom_load_addr_aligned, ((u32)phstart)
      // & MMU_FLASH_MASK, 64, drom_page_count);

      // printf("  move end data load addr %x  from %x count %d
      // ret=%d\n\r",drom_load_addr_aligned,phstart,drom_page_count,rc);

      // memset(vaddr, 0, shdr[i].sh_size);
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    } else if ((shdr[i].sh_type & SHT_PROGBITS == SHT_PROGBITS) &&
               (shdr[i].sh_flags & SHF_ALLOC == SHF_ALLOC) &&
               (shdr[i].sh_flags & SHF_WRITE == SHF_WRITE)) {
      u32* start = shdr[i].sh_offset;
      u32* vaddr = shdr[i].sh_addr;
      print_triplet_hex("load shdr ", (u32)start, (u32)vaddr, shdr[i].sh_size);
      u32* phstart = (u32)elf + shdr[i].sh_offset;
      u32 ret = disk_read_lba(phstart, vaddr, shdr[i].sh_size);
      print_label_hex("ret ", ret);

      // memset(vaddr, 0, shdr->sh_size);
      // memmove32(phstart, vaddr, shdr[i].sh_size);

      // u32 drom_load_addr_aligned = (u32)vaddr & MMU_FLASH_MASK;
      // u32 drom_page_count=(shdr[i].sh_size + ((u32)vaddr - (((u32)vaddr) &
      // MMU_FLASH_MASK)) + MMU_BLOCK_SIZE - 1) / MMU_BLOCK_SIZE; int rc =
      // cache_flash_mmu_set(0, 0,drom_load_addr_aligned , ((u32)phstart) &
      // MMU_FLASH_MASK, 64, drom_page_count);

      // rc |= cache_flash_mmu_set(1, 0, drom_load_addr_aligned, ((u32)phstart)
      // & MMU_FLASH_MASK, 64, drom_page_count);

      // printf("  move end data load addr %x  from %x count %d
      // ret=%d\n\r",drom_load_addr_aligned,phstart,drom_page_count,rc);
    }
  }
}

void print_hex(u32* addr) {
  for (int x = 0; x < 16; x++) {
    print_hex32(addr[x]);
    uart_send_ch(' ');
  }
  uart_send_ch('\n');
}

void* load_kernel() {
#ifdef KERNEL_BIN
  print_string((const unsigned char*)"load kernel bin\n");
  print_string((const unsigned char*)"bin kernel\n");
  return elf;
#else
  Elf32_Ehdr header;
  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)&header;
  print_string((const unsigned char*)"read kernel header\n");
  disk_read_lba(KERNEL_FLASH_ADDR, elf_header, sizeof(Elf32_Ehdr));
  print_label_hex("elf header ", (u32)elf_header);
  u32 magic = *(u32*)&elf_header->e_ident;
  print_label_hex("elf magic ", magic);
  if (magic == 0x464c457f) {
    print_string((const unsigned char*)"load elf kernel\n");
    // printf("header: ");
    // printf("type:%d\n\r", *(u32*)&elf_header->e_type);
    // printf("e_machine:%d\n\r", elf_header->e_machine);
    // printf("e_entry:%x\n\r", elf_header->e_entry);
    // printf("e_phoff:%x\n\r", elf_header->e_phoff);
    // printf("e_shoff:%x\n\r", elf_header->e_shoff);
    // printf("e_ehsize:%x\n\r", elf_header->e_ehsize);
    // printf("e_phentsize:%x\n\r", elf_header->e_phentsize);
    load_elf(elf_header);
    return elf_header->e_entry;
  } else {
    print_string((const unsigned char*)"raw kernel entry\n");
    print_string((const unsigned char*)"bin kernel\n");
    return KERNEL_BASE;
  }
#endif
}

// start kernel
void start_kernel() {
#ifdef SINGLE_KERNEL
  // get_segment();
  extern void kstart(int argc, char* argv[], char** envp);
  entry start = kstart;
  kernel_envp[0] = (char*)&boot_data;
  kernel_envp[1] = 0;
  boot_jump_to_kernel((kernel_entry_fn)start, 0, 0, kernel_envp);
#else
  cache_read_disable(0);
  cache_flush(0);

  for (int i = 0; i < DPORT_FLASH_MMU_TABLE_SIZE; i++) {
    DPORT_PRO_FLASH_MMU_TABLE[i] = DPORT_FLASH_MMU_TABLE_INVALID_VAL;
  }
  boot_info->kernel_entry = load_kernel();
  entry start = boot_info->kernel_entry;
  print_label_hex("kernel entry ", (u32)boot_info->kernel_entry);
  DPORT_REG_CLR_BIT(
      DPORT_PRO_CACHE_CTRL1_REG,
      (DPORT_PRO_CACHE_MASK_IRAM0) | (DPORT_PRO_CACHE_MASK_IRAM1 & 0) |
          (DPORT_PRO_CACHE_MASK_IROM0 & 0) | DPORT_PRO_CACHE_MASK_DROM0 |
          DPORT_PRO_CACHE_MASK_DRAM1);

  DPORT_REG_CLR_BIT(
      DPORT_APP_CACHE_CTRL1_REG,
      (DPORT_APP_CACHE_MASK_IRAM0) | (DPORT_APP_CACHE_MASK_IRAM1 & 0) |
          (DPORT_APP_CACHE_MASK_IROM0 & 0) | DPORT_APP_CACHE_MASK_DROM0 |
          DPORT_APP_CACHE_MASK_DRAM1);

  cache_read_enable(1);

  kernel_envp[0] = (char*)&boot_data;
  kernel_envp[1] = 0;
  boot_jump_to_kernel((kernel_entry_fn)start, 0, 0, kernel_envp);
#endif
}
