#include "mboot32.h"
#include "util32.h"

typedef uint16_t multiboot_uint16_t;
typedef uint32_t multiboot_uint32_t;
typedef uint64_t multiboot_uint64_t;

/* The symbol table for a.out. */
typedef struct _multiboot_aout_symbol_table {
    multiboot_uint32_t tabsize;
    multiboot_uint32_t strsize;
    multiboot_uint32_t addr;
    multiboot_uint32_t reserved;
} multiboot_aout_symbol_table_t;
     
/* The section header table for ELF. */
typedef struct _multiboot_elf_section_header_table {
    multiboot_uint32_t num;
    multiboot_uint32_t size;
    multiboot_uint32_t addr;
    multiboot_uint32_t shndx;
} multiboot_elf_section_header_table_t;

typedef struct _multiboot_info {
    /* Multiboot info version number */
    multiboot_uint32_t flags;

    /* Available memory from BIOS */
    multiboot_uint32_t mem_lower;
    multiboot_uint32_t mem_upper;

    /* "root" partition */
    multiboot_uint32_t boot_device;

    /* Kernel command line */
    multiboot_uint32_t cmdline;

    /* Boot-Module list */
    multiboot_uint32_t mods_count;
    multiboot_uint32_t mods_addr;

    union
    {
     multiboot_aout_symbol_table_t aout_sym;
     multiboot_elf_section_header_table_t elf_sec;
    } u;

    /* Memory Mapping buffer */
    multiboot_uint32_t mmap_length;
    multiboot_uint32_t mmap_addr;

    /* Drive Info buffer */
    multiboot_uint32_t drives_length;
    multiboot_uint32_t drives_addr;

    /* ROM configuration table */
    multiboot_uint32_t config_table;

    /* Boot Loader Name */
    multiboot_uint32_t boot_loader_name;

    /* APM table */
    multiboot_uint32_t apm_table;

    /* Video */
    multiboot_uint32_t vbe_control_info;
    multiboot_uint32_t vbe_mode_info;
    multiboot_uint16_t vbe_mode;
    multiboot_uint16_t vbe_interface_seg;
    multiboot_uint16_t vbe_interface_off;
    multiboot_uint16_t vbe_interface_len;
} multiboot_info_t;

typedef struct _multiboot_mmap_entry {
    multiboot_uint32_t size;
    //multiboot_uint64_t addr;
    multiboot_uint32_t addr_l, addr_h;
    //multiboot_uint64_t len;
    multiboot_uint32_t len_l, len_h;
    #define MULTIBOOT_MEMORY_AVAILABLE              1
    #define MULTIBOOT_MEMORY_RESERVED               2
    multiboot_uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t ;

typedef struct _multiboot_mod_list {
    /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
    multiboot_uint32_t mod_start;
    multiboot_uint32_t mod_end;

    /* Module command line */
    multiboot_uint32_t cmdline;

    /* padding to take it to 16 bytes (must be zero) */
    multiboot_uint32_t pad;
} multiboot_mod_list_t;

char* mboot_get_cmdline(uint32_t multiboot_raw_ptr) {
    multiboot_info_t *mboot_info = (multiboot_info_t*)multiboot_raw_ptr;
    
    if (mboot_info->flags & BIT_2) {
        return (char*)mboot_info->cmdline;
    }
    
    return NULL;
}

uint32_t mboot_get_module_count(uint32_t multiboot_raw_ptr) {
    multiboot_info_t *mboot_info = (multiboot_info_t*)multiboot_raw_ptr;
    
    if (mboot_info->flags & BIT_3) {
        return mboot_info->mods_count;
    }
    
    return 0;
}

uint32_t mboot_get_module(uint32_t multiboot_raw_ptr, uint32_t mod_num, mboot_modinfo_t *info) {
    multiboot_info_t *mboot_info = (multiboot_info_t*)multiboot_raw_ptr;
    
    if ((mboot_info->flags & BIT_3) && (mboot_info->mods_count > mod_num)) {
        multiboot_mod_list_t *mod = (multiboot_mod_list_t*)(mboot_info->mods_addr + mod_num * sizeof(multiboot_mod_list_t));
        info->addr = (void*)mod->mod_start;
        info->size = (mod->mod_end - mod->mod_start);
        for (uint32_t i=0; i<64; i++) {
            char c = ((char*)mod->cmdline)[i];
            info->cmdline[i] = c;
            if (c == '\0') break;
        }
        return 0;
    }
    
    return 1;
}

uint32_t mboot_mmap_begin(uint32_t multiboot_raw_ptr, mboot_mmap_root_t *root) {
    multiboot_info_t *mboot_info = (multiboot_info_t*)multiboot_raw_ptr;
    
    if (mboot_info->flags & BIT_6) {
        root->next = (void*)mboot_info->mmap_addr;
        root->end = (void*)(mboot_info->mmap_addr + mboot_info->mmap_length);
        
        return 0;
    }
    
    return 1;
}

uint32_t mboot_mmap_get(mboot_mmap_root_t *root, mboot_mmap_entry_t *entry) {
    *entry = *(mboot_mmap_entry_t*)(root->next); //mboot_mmap_entry_t MUST be binary compatible with
                                                 //multiboot_mmap_entry_t
    
    root->next += entry->size+4;
    
    return (root->next >= root->end);
}
