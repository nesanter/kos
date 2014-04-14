#ifdef __MODE_KERNEL32
#error "THIS HEADER IS NOT STAGE1 COMPATIBLE!"
#endif

#ifndef _KERNEL32_COMPAT_H
#define _KERNEL32_COMPAT_H

typedef uint64_t bits64_t;



#define KERNEL32_MAX_BOOT_MODULES 8
#define KERNEL32_MAX_MOD_OPT_LENGTH 255
#define KERNEL32_MAX_MOD_NAME_LENGTH 31

typedef struct _modinfo_flags {
    uint32_t present:1;
    uint32_t kernel64:1;
    uint32_t _reserved:30;
} modinfo_flags_t;

typedef struct _modinfo {
    uint32_t addr;
    uint32_t size;
    uint32_t entry_low;
    uint32_t entry_high;
    char name[KERNEL32_MAX_MOD_NAME_LENGTH + 1];
    char opts[KERNEL32_MAX_MOD_OPT_LENGTH + 1];
    modinfo_flags_t flags;
} modinfo_t;


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

#endif
