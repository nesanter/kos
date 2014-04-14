#ifndef _HANDOFF_H
#define _HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#ifdef __MODE_KERNEL32

#include "kernel32.h"
#include "mod32.h"
#include "mem32.h"

#else

#include "kernel32_compat.h" //autogenerated

#endif

typedef union _kernel_cmddline_flags {
    struct {
        uint32_t verbose:1;
        uint32_t verbose_mmap:1;
        uint32_t _reserved:30;
    };
    uint32_t _all;
} kernel_cmdline_flags_t;

typedef struct _kernel_handoff {
    uint32_t magic[2];
    //gdt_ptr_t gdt_ptr;
    //idt_ptr_t idt_ptr;
    mmap_entry_t mmap[KERNEL32_MAX_MMAP_ENTRIES];
    mmap_entry_t reserved[KERNEL32_MAX_RESERVED_ENTRIES];
    modinfo_t mod_table[KERNEL32_MAX_BOOT_MODULES];
    uint32_t mmap_entry_num;
    uint32_t reserved_entry_num;
    uint32_t mod_entry_num;
    kernel_cmdline_flags_t flags;
} kernel_handoff_t;

#define HANDOFF_T_PAGESIZE ((sizeof(kernel_handoff_t) % 4096) ? (sizeof(kernel_handoff_t) / 4096 + 1) : (sizeof(kernel_handoff_t) / 4096))

#endif
