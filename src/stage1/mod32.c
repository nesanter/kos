#include "kernel32.h"
#include "mboot32.h"
#include "mod32.h"
#include "mem32.h"
#include "terminal32.h"
#include "util32.h"
#include "elf.h"
#include "ksym.h"

void mod32_init() {
    kernel32_modules_loaded = 0;
    modinfo_t zero_mod = {0};
    for (uint32_t i=0; i<KERNEL32_MAX_BOOT_MODULES; i++) {
        kernel32_modules_table[i] = zero_mod;
    }
}

/* this first stage of module loading exists to determine
 * how much space the raw modules occupy in memory
 * so that we can begin loading beyond that point
 * 
 * it also records the module command line, address, size, etc.
 */
uint32_t mod32_register(mboot_modinfo_t *old_minfo, uint32_t verbose) {
    modinfo_t minfo;
    if (verbose) {
        kterm_write("[note] module: addr=");
        kterm_write_ui32hx(old_minfo->addr);
        kterm_write(" size=");
        kterm_write_ui32hx(old_minfo->size);
        kterm_write(" cmdline=");
        kterm_write(old_minfo->cmdline);
        kterm_write_line();
    }
    /*
    uint32_t i = 0;
    while (1) {
        char c = old_minfo->cmdline[i];
        minfo.name[i] = c;
        if (c == '\0' || c == ' ') {
            break;
        }
        i++;
    }
    
    for (uint32_t ii=i; ii<sizeof(kernel32_modules_root_name); ii++) {
        minfo.name[ii] = '\0';
    }
    
    uint32_t j = 0;
    while (1) {
        char c = old_minfo->cmdline[i++];
        minfo.opts[j++] = c;
        if (c == '\0')
            break;
    }
    */
    
    uint32_t i = 0;
    while (1) {
        char c = old_minfo->cmdline[i];
        minfo.opts[i++] = c;
        if (c == '\0')
            break;
        if (i == KERNEL32_MAX_MOD_OPT_LENGTH) {
            minfo.opts[i] = '\0';
            break;
        }
    }
    
    minfo.addr = (uint32_t)old_minfo->addr;
    minfo.size = old_minfo->size;
    
    minfo.flags.present = 1;
    
    /*
    if (strcmp_fix((uint32_t*)kernel32_modules_root_name, (uint32_t*)minfo.name, 8)) {
        kernel32_modules_table[0] = minfo;
    } else {
        kernel32_modules_table[kernel32_modules_loaded++] = minfo;
    }
    */
    
    kernel32_modules_table[kernel32_modules_loaded++] = minfo;
    
    if (minfo.addr < kernel32_modules_ptr[0] || kernel32_modules_ptr[0] == 0)
        kernel32_modules_ptr[0] = minfo.addr;
    
    if (minfo.addr + minfo.size > kernel32_modules_ptr[1])
        kernel32_modules_ptr[1] = minfo.addr + minfo.size;
    
    return 0;
}

void mod32_set_root_name(const char *name) {
    uint32_t i;
    for (i=0; i<sizeof(kernel32_modules_root_name); i++) {
        char c = *name++;
        kernel32_modules_root_name[i] = c;
        if (c == '\0')
            break;
    }
    for (; i<sizeof(kernel32_modules_root_name); i++) {
        kernel32_modules_root_name[i] = '\0';
    }
}


/* this parses all the elf headers of the modules
 * in search of kernel64, which it then loads to dest
 * 
 * it does not build any sort of kernel symbol table
 * or load any other boot modules
 */
void* mod32_load_k64(void *dest, uint32_t space, kernel64_isr_ptrs_t *isr_ptrs, uint32_t *kmodnum, uint32_t verbose) {
    //ksym_table_32_t *ksym_table = (ksym_table_32_t*)dest;
    
    //if (sizeof(ksym_table_32_t) >= 4096) {
    //    kterm_write("[warn] sizeof(ksym_table_32_t) >= 4096\n");
    //}
    
    //dest += sizeof(ksym_table_32_t);
    //dest += (4096 - (uint32_t)dest % 4096);
    
    
    void* destmax = dest + space;
    
    uint32_t knum = (uint32_t)-1;
    
    for (uint32_t i=0; i<kernel32_modules_loaded; i++) {
        if (verbose) {
            kterm_write("[note] checking module ");
            kterm_write_ui32d(i);
            kterm_write_line();
        }
        
        //first the elf header
        elf64_header_t *elf_header = (elf64_header_t*)kernel32_modules_table[i].addr;
        
        //sanity checks
        if (elf_header->e_ident.ei_mag_all != ELF64_MAGIC) {
            kterm_write("[warn] bad elf magic ");
            kterm_write_ui32h(elf_header->e_ident.ei_mag_all);
            kterm_write_line();
            return NULL;
        }
        if (elf_header->e_ident.ei_class != ELF64_ELFCLASS64) {
            kterm_write("[warn] elf e_ident.ei_class not ELFCLASS64\n");
            return NULL;
        }
        if (elf_header->e_ident.ei_data != ELF64_ELFDATA2LSB) {
            kterm_write("[warn] elf e_ident.ei_data not ELFDATA2LSB\n");
            return NULL;
        }
        if (elf_header->e_ident.ei_version != ELF64_EV_CURRENT) {
            kterm_write("[warn] elf e_ident.ei_version not EV_CURRENT\n");
            return NULL;
        }
        if (elf_header->e_ident.ei_osabi != ELF64_ELFOSABI_SYSV) {
            kterm_write("[warn] elf e_ident.ei_osabi not ELFOSABI_SYSV\n");
            return NULL;
        }
        if (elf_header->e_type != ELF64_ET_EXEC) {
            kterm_write("[warn] elf e_type not ET_EXEC\n");
            return NULL;
        }
        if (elf_header->e_version != ELF64_EV_CURRENT) {
            kterm_write("[warn] elf e_version not EV_CURRENT\n");
            return NULL;
        }
        
        if (verbose) {
            kterm_write("[note] elf header is sane\n");
        }
        
        //load
        /*
        if (verbose) {
            kterm_write("[note] program header table at ");
            kterm_write_ui32hx(elf_header->e_phoff & 0xFFFFFFFF);
            kterm_write(" (");
            kterm_write_ui16hx(elf_header->e_phnum);
            kterm_write(" x ");
            kterm_write_ui16hx(elf_header->e_phentsize);
            kterm_write(" bytes)\n");
            kterm_write("[note] section header table at ");
            kterm_write_ui32hx(elf_header->e_shoff & 0xFFFFFFFF);
            kterm_write(" (");
            kterm_write_ui16hx(elf_header->e_shnum);
            kterm_write(" x ");
            kterm_write_ui16hx(elf_header->e_shentsize);
            kterm_write(" bytes)\n");
        }
        */
        //identify module name
        
        void *sptr = (void*)((void*)elf_header + (uint32_t)elf_header->e_shoff);
        
        //were looknig for the modinfo section
        //which always begins with 0xFF 0x4D 0x49 0x44 0x45 0x4E 0x54 0x00
        //this will always be in a PROGBITS (sh_type 1) section
        
        char mident_keyword[8] = {0xFF, 0x4D, 0x49, 0x44, 0x45, 0x4E, 0x54, 0x00};
        
        for (uint32_t sh=0; sh<elf_header->e_shnum; sh++) {
            elf64_section_header_t *shptr = (elf64_section_header_t*)sptr;
            
            if (shptr->sh_type == ELF64_SHT_PROGBITS) {
                char *entry = (char*)((void*)elf_header + shptr->sh_offset);
                
                if (strcmp_fix((uint32_t*)entry, (uint32_t*)mident_keyword, 2)) {
                    //this is the mident section
                    uint32_t j = 0;
                    while (entry[8+j] && j < KERNEL32_MAX_MOD_NAME_LENGTH) {
                        kernel32_modules_table[i].name[j] = entry[8+j];
                        j++;
                    }
                    kernel32_modules_table[i].name[j] = '\0';
                    if (j == KERNEL32_MAX_MOD_NAME_LENGTH) {
                        kterm_write("[warn] module name truncated\n");
                    }
                    if (verbose) {
                        kterm_write("[note] module name ");
                        kterm_write(kernel32_modules_table[i].name);
                        kterm_write_line();
                    }
                    if (strcmp(kernel32_modules_table[i].name, kernel32_modules_root_name)) {
                        //we found kernel64
                        knum = i;
                        *kmodnum = i;
                        
                        kernel32_modules_table[i].flags.kernel64 = 1;
                        
                        if (j % 8)
                            j += 8 - (j % 8);
                        
                        uint32_t *entry32 = (uint32_t*)&entry[8+j];
                        
                        if (!(entry32[0] == 0x525349FF && entry32[1] == 0x53525450)) {
                            kterm_write("[warn] ISRPTRS magic not found in modinfo of module identified as kernel64\n");
                            //kterm_write("0x525349FF 0x53525450\n");
                            kterm_write_ui32hx(entry32[0]);
                            kterm_write(" ");
                            kterm_write_ui32hx(entry32[1]);
                            kterm_write_line();
                            return NULL;
                        }
                        
                        isr_ptrs->gp_high = entry32[2];
                        isr_ptrs->gp_low = entry32[3];
                        isr_ptrs->pf_high = entry32[4];
                        isr_ptrs->pf_low = entry32[5];
                        /*
                        uint32_t extra = entry32[7];
                        
                        kterm_write("[note] extra is ");
                        kterm_write_ui32d(extra);
                        kterm_write_line();
                        
                        kterm_write("[note] bootstrap64 is ");
                        kterm_write_ui32hx(kernel32_bootstrap64);
                        kterm_write_line();
                        */
                        /*
                        for (uint32_t x=0; x<extra; x++) {
                            kterm_write_ui32hx(entry[8+x]);
                            kterm_write_line();
                            kernel32_bootstrap64[x] = entry[8+x];
                        }
                        */
                        break;
                    }
                    
                    
                    
                }
                
            }
            
            sptr += elf_header->e_shentsize;
        }
        
        if (knum < kernel32_modules_loaded) {
            break;
        }
        
    }
    
    if (knum == (uint32_t)-1) {
        kterm_write("[warn] kernel64 not found\n");
        return NULL;
    }
    
    if (verbose) {
        kterm_write("[note] found kernel64\n");
    }
    
    //now we can load kernel64
    elf64_header_t *elf_header = (elf64_header_t*)(uint32_t)kernel32_modules_table[knum].addr;
    
    kernel32_modules_table[knum].entry_low = (uint32_t)(elf_header->e_entry & 0xFFFFFFFF);
    kernel32_modules_table[knum].entry_high = (uint32_t)(elf_header->e_entry >> 32);
        
    void *ptr = (void*)((void*)elf_header + (uint32_t)elf_header->e_phoff);
        
    for (uint32_t ph=0; ph<elf_header->e_phnum; ph++) {
        elf64_program_header_t *phptr = (elf64_program_header_t*)ptr;
                    
        if (phptr->p_type == ELF64_PT_LOAD) {
            void *secptr = (void*)((void*)elf_header + (uint32_t)phptr->p_offset);
            
            if ((uint32_t)dest % (uint32_t)phptr->p_align > 0)
                dest += phptr->p_align - ((uint32_t)dest % (uint32_t)phptr->p_align);
            
            if (verbose) {
                kterm_write("[note] expanding section (");
                kterm_write_ui32hx(secptr);
                kterm_write(" -> ");
                kterm_write_ui32hx(dest);
                kterm_write(" (");
                kterm_write_ui32hx(phptr->p_memsz);
                kterm_write(" bytes)\n");
            }
            
            if (dest + phptr->p_memsz > destmax) {
                kterm_write("[warn] insufficient memory at load target\n");
                return NULL;
            }
            
            uint32_t j;
            
            for (j=0; j<(phptr->p_filesz >> 2); j++) {
                *(uint32_t*)dest = *(uint32_t*)secptr;
                secptr += 4;
                dest += 4;
            }
            for (; j<(phptr->p_memsz >> 2); j++) {
                *(uint32_t*)dest = 0;
                dest += 4;
            }
            
        } else {
            if (verbose)
                kterm_write("[note] ignoring elf program header entry not marked as PT_LOAD\n");
        }
        
        ptr += elf_header->e_phentsize;
        
    }
    
    /*  this returns a pointer to free space after
     *  the end of the modules (e.g. dest + space_used_loading)
     *  the handoff struct will end up there
     */
    return ADJUST_PTR(dest, 4096);
}



