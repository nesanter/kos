#ifndef __MODE_KERNEL32
#error "THIS HEADER IS STAGE1 ONLY!"
#endif

#ifndef _MEM32_H
#define _MEM32_H

#include <stddef.h>
#include <stdint.h>

#include "mboot32.h"

typedef struct _bits64 {
    uint32_t l,h;
} bits64_t;

//##KERNEL32_COMPAT_START

#define KERNEL32_MAX_MMAP_ENTRIES 16
#define KERNEL32_MAX_RESERVED_ENTRIES 2

typedef struct _mmap_entry {
    /*
    uint64_t address;
    uint64_t length;
    */
    bits64_t address;
    bits64_t length;
} mmap_entry_t;

typedef struct _gdt_ptr {
    uint16_t limit;
    uint32_t base_high;
    uint32_t base_low;
} __attribute__((aligned(8))) gdt_ptr_t;

typedef struct _idt_ptr {
    uint16_t limit;
    uint32_t base_high;
    uint32_t base_low;
} __attribute__((aligned(8))) idt_ptr_t;

//##KERNEL32_COMPAT_END

typedef struct _kernel64_isr_ptrs {
    uint32_t gp_low;
    uint32_t gp_high;
    uint32_t pf_low;
    uint32_t pf_high;
    uint32_t df_low;
    uint32_t df_high;
    uint32_t i80_low;
    uint32_t i80_high;
} kernel64_isr_ptrs_t;

mmap_entry_t mmap[KERNEL32_MAX_MMAP_ENTRIES];
uint32_t mmap_entry_count;

uint32_t mmap_build(uint32_t multiboot_raw_ptr, uint32_t verbose);
void* mem32_pick_target();

uint32_t mem32_build_gdt(gdt_ptr_t *gdt_ptr, void* dest, uint32_t *size);
uint32_t mem32_build_idt(idt_ptr_t *idt_ptr, void* dest, uint32_t *size, kernel64_isr_ptrs_t *isr_ptrs);

uint32_t mem32_check(uint32_t addr, uint32_t size);
uint32_t mem32_space(uint32_t addr);

//paging
uint32_t mem32_setup_early_paging(void **page_table_ptr_ptr, void **end_ptr, void *kernel64_start, void *kernel64_end, uint32_t verbose);

#define ADJUST_PTR(ptr,mult) ((uint32_t)ptr % mult ? ptr + (mult - ((uint32_t)ptr % mult)) : ptr)


extern char _end[];

#endif
