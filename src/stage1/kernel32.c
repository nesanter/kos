/*  kernel32_main is the entrypoint specified in boot.asm
 *  the cpu has been placed in protected mode by grub
 *  and will stay that way until we call kernel32_finalize()
 *  which is also defined in boot.asm
 *  before we do this, stage2 must prepared to receive us
 */

#include "kernel32.h"
#include "terminal32.h"
#include "cpuid32.h"
#include "util32.h"
#include "mboot32.h"
#include "mod32.h"
#include "mem32.h"
#include "ksym.h"

uint32_t parse_cmdline(const char *cmdline, kernel32_cmdline_t *opts);
uint32_t check_long_mode_capable();
uint32_t check_tagged_tlb();

//extern void kernel32_finalize(void *bootstrap_ptr_low, void* bootstrap_ptr_high,
//    void* handoff_ptr, cr0_t cr0_new, cr4_t cr4_new, uint32_t cr3_new);
//extern void kernel32_finalize(void* handoff_ptr, cr0_t cr0_new, cr4_t cr4_new, uint32_t cr3_new);
extern void kernel32_finalize(void* handoff_ptr, /*cr0_t cr0_new, cr4_t cr4_new,*/ uint32_t cr3_new);

extern gdt_ptr_t kernel32_gdt;
extern idt_ptr_t kernel32_idt;

void kernel32_main(uint32_t multiboot_ptr) {
    kterm_initialize();
    
    //step 1: sanity check
    if (check_long_mode_capable()) {
        kterm_write("[fail] sanity check failed: long mode not supported\n");
        kernel32_hang();
    } else {
        kterm_write("[note] sanity check passed: long mode is supported\n");
    }
    
    //step 1b: PCID sanity check (this may become optional in future)
    if (check_tagged_tlb()) {
        kterm_write("[fail] sanity check failed: process context identifiers not supported\n");
        kernel32_hang();
    } else {
        kterm_write("[note] sanity check passed: process context identifiers supported\n");
    }
    
    //Since we have long mode:
    //kernel32_enable_sse();
    
    //step 2: multiboot parsing
    
    //step 2a: cmdline
    const char *cmdline = mboot_get_cmdline(multiboot_ptr);
    
    kernel32_cmdline_t opts;
    
    if (!cmdline) {
        kterm_write("[warn] multiboot info reports no kernel cmdline\n");
    } else {
        /*
        kterm_write("[note] cmdline: '");
        kterm_write(cmdline);
        kterm_write("'\n");
        */
        
        if (parse_cmdline(cmdline, &opts)) {
            kterm_write("[fail] failed to parse kernel command line\n");
            kernel32_hang();
        }
    }
    
    //step 2b: get memory map
    if (mmap_build(multiboot_ptr, opts.flags.verbose_mmap)) {
        kterm_write("[fail] cannot continue without mmap\n");
        kernel32_hang();
    }
    
    //step 3: locate and load kernel64 module(s)
    
    if (opts.kernel64_name[0] == '\0') {
        kterm_write("[note] option kernel64 not present on cmdline\n[note] reverting to default\n");
        memcpy(opts.kernel64_name, DEFAULT_KERNEL64, strlen(DEFAULT_KERNEL64));
    }
    
    if (opts.flags.verbose) {
        kterm_write("[note] kernel64 module is named ");
        kterm_write(opts.kernel64_name);
        kterm_write_line();
    }
    
    mod32_init();
    mod32_set_root_name(opts.kernel64_name);
    
    uint32_t modcount = mboot_get_module_count(multiboot_ptr);
    
    if (modcount == -1) {
        kterm_write("[fail] multiboot info reports no module table\n");
        kernel32_hang();
    }
    
    if (opts.flags.verbose) {
        kterm_write("[note] module count: ");
        kterm_write_ui32d(modcount);
        kterm_write_line();
    }
    
    if (modcount == 0) {
        kterm_write("[warn] module count is zero\n");
    }
    
    if (modcount > KERNEL32_MAX_BOOT_MODULES) {
        kterm_write("[warn] only first ");
        kterm_write_ui32d(KERNEL32_MAX_BOOT_MODULES);
        kterm_write(" modules will be loaded\n");
        kterm_write("[warn] consider increasing KERNEL32_MAX_BOOT_MODULES\n");
        modcount = KERNEL32_MAX_BOOT_MODULES;
    }
    
    for (uint32_t i=0; i<modcount; i++) {
        mboot_modinfo_t minfo;
        if (mboot_get_module(multiboot_ptr, i, &minfo)) {
            kterm_write("[warn] error fetching module ");
            kterm_write_ui32d(i);
            kterm_write(" from multiboot info\n");
            kterm_write_line();
            continue;
        }
        
        mod32_register(&minfo, opts.flags.verbose);
    }
    
    /*
    if (!kernel32_modules_table[0].flags.present) {
        kterm_write("[fail] kernel64 (");
        kterm_write(kernel32_modules_root_name);
        kterm_write(") not found in boot modules\n");
        kernel32_hang();
    }
    */
    
    //with a memory map + knowledge of where the boot modules are
    //we know now were "safe" memory is, so we can load
    
    if (opts.flags.verbose) {
        kterm_write("[note] kernel32 end addr=");
        kterm_write_ui32hx(_end);
        kterm_write("\n[note] module(s) addr=");
        kterm_write_ui32hx(kernel32_modules_ptr[0]);
        kterm_write(" to ");
        kterm_write_ui32hx(kernel32_modules_ptr[1]);
        kterm_write_line();
    }
    
    void* load_target = mem32_pick_target();
    
    if (opts.flags.verbose) {
        kterm_write("[note] load target chosen at ");
        kterm_write_ui32hx(load_target);
        kterm_write_line();
    }
    
    if (!load_target) {
        kterm_write("[fail] unable to find space for kernel64\n");
        kernel32_hang();
    }
    
    kernel64_isr_ptrs_t isr_ptrs;
    
    uint32_t kmodnum;
    
    void* load_target2 = mod32_load_k64(load_target, mem32_space((uint32_t)load_target), &isr_ptrs, &kmodnum, opts.flags.verbose);
    
    if (!load_target2) {
        kterm_write("[fail] error loading kernel64 to target\n");
        kernel32_hang();
    }
    
    if (opts.flags.verbose) {
        kterm_write("kernel64 isr ptr (gp/pf): ");
        kterm_write_ui32hx(isr_ptrs.gp_low);
        kterm_write_ui32h(isr_ptrs.gp_high);
        kterm_write("/");
        kterm_write_ui32hx(isr_ptrs.pf_low);
        kterm_write_ui32h(isr_ptrs.pf_high);
        kterm_write_line();
    }
    
    //populate handoff
    
    //step ??: determine kernel handover location
    //this must happen after we've relocated the modules to a known address
    kernel_handoff_t *handoff = (kernel_handoff_t*)load_target2;
    
    load_target2 += sizeof(kernel_handoff_t);
    
    handoff->magic[0] = 0x534E524B;
    handoff->magic[1] = 0x4C455454;
    
    handoff->flags = opts.flags;
    
    for (uint32_t i=0; i<mmap_entry_count; i++) {
        //handoff->mmap_addr[i] = mmap[i].address;
        //handoff->mmap_length[i] = mmap[i].length;
        handoff->mmap[i] = mmap[i];
    }
    handoff->mmap_entry_num = mmap_entry_count;
    
    for (uint32_t i=0; i<kernel32_modules_loaded; i++) {
        handoff->mod_table[i] = kernel32_modules_table[i];
    }
    handoff->mod_entry_num = kernel32_modules_loaded;
    
    //now we find a safe place to put the gdt
    
    //is 0x1000 free and at least 1 page long?
    void* ptr;
    
    if (mem32_check(0x1000, 0x1000)) {
        //have to put this in higher memory
        //ptr = load_target2 + (4096 - ((uint32_t)load_target2 % 4096));
        ptr = ADJUST_PTR(load_target2, 0x1000);
    } else {
        ptr = (void*)0x1000;
    }
    
    if (opts.flags.verbose) {
        kterm_write("[note] secondary load target chosen at ");
        kterm_write_ui32hx(ptr);
        kterm_write_line();
    }
    
    //setup gdt
    
    uint32_t sz;
    if (mem32_build_gdt(&kernel32_gdt, ptr, &sz)) {
        kterm_write("[fail] error building gdt\n");
        kernel32_hang();
    }
    
    ptr = ADJUST_PTR((ptr+sz), 8);
    
    kterm_write("ptr = ");
    kterm_write_ui32hx(ptr);
    kterm_write("; sz = ");
    kterm_write_ui32d(sz);
    kterm_write_line();
    
    if (mem32_build_idt(&kernel32_idt, ptr, &sz, &isr_ptrs)) {
        kterm_write("[fail] error building idt\n");
        kernel32_hang();
    }
    
    ptr = ADJUST_PTR((ptr+sz), 0x1000);
    
    kterm_write("ptr = ");
    kterm_write_ui32hx(ptr);
    kterm_write("; sz = ");
    kterm_write_ui32d(sz);
    kterm_write_line();
    
    //setup paging
    if (mem32_setup_early_paging(&ptr, load_target, load_target2, opts.flags.verbose)) {
        kterm_write("[fail] error building page tables\n");
        kernel32_hang();
    }
    
    
    uint32_t cr3_new = (uint32_t)ptr;
    
    /*
    //setup new values of cr0/cr4
    cr0_t cr0_new = read_cr0();
    cr4_t cr4_new = read_cr4();
    
    //if (sizeof(cr0_t) != 4)
    //    kterm_write("bad size cr0");
    //if (sizeof(cr4_t) != 4)
    //    kterm_write("bad size cr4");
    
    cr0_new.mp = 1;
    cr0_new.em = 0;
    cr0_new.ne = 1;
    cr0_new.pg = 1;
    cr4_new.pae = 1;
    cr4_new.osfxsr = 1;
    cr4_new.osxmmexcpt = 1;
    */
    //now we're ready!
    
    //void *bootstrap_ptr = (void*)((ksym_table_32_t*)load_target)->bootstrap64_ptr;
    
    if (opts.flags.verbose) {
        kterm_write("[note] calling kernel32_finalize\n");
        //kterm_write_ui32hx(cr3_new);
        //kterm_write_line();
        /*
        kterm_write_ui32hx(kernel32_modules_table[kmodnum].entry_low);
        kterm_write(":");
        kterm_write_ui32h(kernel32_modules_table[kmodnum].entry_high);
        kterm_write(", ");
        kterm_write_ui32hx(load_target2);
        kterm_write(")");
        kterm_write_line();
        */
    }
    
    //kernel32_finalize(kernel32_modules_table[kmodnum].entry_low,
    //    kernel32_modules_table[kmodnum].entry_high, load_target2,
    //    cr0_new, cr4_new, cr3_new);
    kernel32_finalize(load_target2, /*cr0_new, cr4_new,*/ cr3_new);
}

uint32_t check_long_mode_capable() {
    cpuid_precheck_info_t* pi = kernel32_cpuid_precheck();
    
    if (!pi->valid)
        return 1;
    
    if (pi->max_high < 1)
        return 1;
    
    /*
    switch (pi->vendor) {
        case CPUID_VID_INTEL:
            kterm_header[18] = 'i';
            kterm_header[19] = 'n';
            kterm_header[20] = 't';
        break;
        case CPUID_VID_AMD:
            kterm_header[18] = 'a';
            kterm_header[19] = 'm';
            kterm_header[20] = 'd';
        break;
        case CPUID_VID_UNKNOWN:
            kterm_header[18] = 'u';
            kterm_header[19] = 'n';
            kterm_header[20] = 'k';
        break;
        default:
            kterm_header[18] = '?';
            kterm_header[19] = '?';
            kterm_header[20] = '?';
        break;
    }
    */
    
    cpuid_info_t info;
    kernel32_cpuid(0x80000001, &info);
    
    if (!(info.edx & BIT_29))
        return 1;
    
    return 0;
}

uint32_t check_tagged_tlb() {
    cpuid_info_t info;
    kernel32_cpuid(0x1, &info);
    
    return !(info.ecx & BIT_17);
}
uint32_t parse_cmdline(const char *cmdline, kernel32_cmdline_t *opts) {
    char option[32];
    char option_val[32];
    uint32_t option_len = 0;
    uint32_t option_val_len = 0;
    uint32_t is_val = 0;
    
    for (uint32_t i=0; i<32; i++)
        opts->kernel64_name[i] = '\0';
    opts->flags._all = 0;
    
    while (1) {
        char c = *cmdline++;
        
        if (c == ' ' || c == ';' || c == '\0') {
            if (option_len > 0) {
                
                option[option_len] = '\0';
                option_val[option_val_len] = '\0';
                
                if (strcmp(option, "kernel64")) {
                    memcpy(opts->kernel64_name, option_val, 32);
                } else if (strcmp(option, "verbose")) {
                    opts->flags.verbose = 1;
                } else if (strcmp(option, "verbose-mmap")) {
                    opts->flags.verbose_mmap = 1;
                } else {
                    kterm_write("[warn] unknown option in command line: ");
                    kterm_write(option);
                    kterm_write_line();
                }
                
                option_len = 0;
                option_val_len = 0;
                is_val = 0;
            }
            if (c == '\0')
                break;
            continue;
        }
        
        if (c == '=') {
            if (is_val) {
                return 1;
            }
            is_val = 1;
            continue;
        }
        
        if (is_val) {
            if (option_val_len == 31) {
                return 1;
            }
            option_val[option_val_len++] = c;
        } else {
            if (option_len == 31) {
                return 1;
            }
            option[option_len++] = c;
        }
        
    }
    
    return 0;
}
