#ifndef __MODE_KERNEL32
#error "THIS HEADER IS STAGE1 ONLY!"
#endif

#ifndef _MBOOT32_H
#define _MBOOT32_H

#include <stddef.h>
#include <stdint.h>

typedef struct _mboot_modinfo {
    void* addr;
    uint32_t size;
    char cmdline[256];
} mboot_modinfo_t;

typedef struct _mboot_mmap_root {
    void* next;
    void* end;
} mboot_mmap_root_t;

typedef struct _mboot_mmap_entry {
    uint32_t size;
    //multiboot_uint64_t addr;
    uint32_t addr_l, addr_h;
    //multiboot_uint64_t len;
    uint32_t len_l, len_h;
    uint32_t type;
} __attribute__((packed)) mboot_mmap_entry_t ;

char* mboot_get_cmdline(uint32_t multiboot_raw_ptr);
uint32_t mboot_get_module_count(uint32_t multiboot_raw_ptr);
uint32_t mboot_get_module(uint32_t multiboot_raw_ptr, uint32_t mod_num, mboot_modinfo_t *info);

uint32_t mboot_mmap_begin(uint32_t multiboot_raw_ptr, mboot_mmap_root_t *root);
uint32_t mboot_mmap_get(mboot_mmap_root_t *root, mboot_mmap_entry_t *entry);

#endif
