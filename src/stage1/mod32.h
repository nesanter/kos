#ifndef __MODE_KERNEL32
#error "THIS HEADER IS STAGE1 ONLY!"
#endif

#ifndef _MOD32_H
#define _MOD32_H

#include <stddef.h>
#include <stdint.h>

#include "mboot32.h"
#include "mem32.h"

//##KERNEL32_COMPAT_START

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
    uint32_t entry;
    char name[KERNEL32_MAX_MOD_NAME_LENGTH + 1];
    char opts[KERNEL32_MAX_MOD_OPT_LENGTH + 1];
    modinfo_flags_t flags;
} modinfo_t;

//##KERNEL32_COMPAT_END

modinfo_t kernel32_modules_table[KERNEL32_MAX_BOOT_MODULES];
uint32_t kernel32_modules_loaded;
char kernel32_modules_root_name[KERNEL32_MAX_MOD_NAME_LENGTH];

uint32_t kernel32_modules_ptr[2];

void mod32_init();
uint32_t mod32_register(mboot_modinfo_t *old_minfo, uint32_t verbose);
void mod32_set_root_name(const char *name);
void* mod32_load_k64(void* dest, uint32_t space, kernel64_isr_ptrs_t *isr_ptrs, uint32_t verbose);

#endif
