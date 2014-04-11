#include "mem32.h"
#include "mod32.h"
#include "terminal32.h"
#include "util32.h"
#include "kernel32.h"

/* GDT */

typedef struct _gdt_segment {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t type_a:1;
    uint8_t type_w:1;
    uint8_t type_e:1;
    uint8_t type_c:1;
    uint8_t system:1;
    uint8_t dpl:2;
    uint8_t present:1;
    uint8_t limit_high:4;
    uint8_t _unused:1;
    uint8_t l:1;
    uint8_t db:1;
    uint8_t g:1;
    uint8_t base_high;
} __attribute__((packed)) gdt_segment_t;

typedef struct _idt_segment {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) idt_segment_t;

uint32_t mmap_build(uint32_t multiboot_raw_ptr, uint32_t verbose) {
    
    mboot_mmap_root_t root;
    
    if (mboot_mmap_begin(multiboot_raw_ptr, &root)) {
        kterm_write("[warn] multiboot info reports no memory map\n");
        return 1;
    }
    
    mboot_mmap_entry_t raw_entry;
    mmap_entry_t entry;
    while (!mboot_mmap_get(&root, &raw_entry)) {
        if (verbose) {
            kterm_write("[note] mmap entry: addr=");
            kterm_write_ui32hx(raw_entry.addr_h);
            kterm_write_ui32h(raw_entry.addr_l);
            kterm_write(" len=");
            kterm_write_ui32hx(raw_entry.len_h);
            kterm_write_ui32h(raw_entry.len_l);
            kterm_write(" type=");
            kterm_write_ui32d(raw_entry.type);
            kterm_write_line();
        }
        
        if (raw_entry.type == 1) {
            entry.address = (uint64_t)raw_entry.addr_l | ((uint64_t)raw_entry.addr_h << 32);
            entry.length = (uint64_t)raw_entry.len_l | ((uint64_t)raw_entry.len_h << 32);
            mmap[mmap_entry_count++] = entry;
        }
    }
    
    return 0;
}

//pick a memory target for handoff + boot modules
//this should be a multiple of 2MB (0x200000)
#define K64_MULTIPLE (0x200000)
void* mem32_pick_target() {
    return (void*)ADJUST_PTR(kernel32_modules_ptr[1], K64_MULTIPLE);
}

uint32_t mem32_build_gdt(gdt_ptr_t *gdt_ptr, void* dest, uint32_t *size) {
    if ((uint32_t)dest % 8)
        return 1;
    
    gdt_segment_t *gdt_table = (gdt_segment_t*)dest;
    
    gdt_segment_t seg_zero = {0};
    gdt_table[0] = seg_zero;
    
    gdt_segment_t seg = {.limit_low = 0xFFFF, .base_low = 0x0000, .base_mid = 0x00,
                         .type_a = 0, .type_w = 1, .type_e = 0, .type_c = 1, .system = 0,
                         .dpl = 0, .present = 1, .limit_high = 0xF, ._unused = 0,
                         .l = 1, .db = 0, .g = 1, .base_high = 0x00};
    gdt_table[1] = seg;
    
    seg.type_c = 1;
    gdt_table[2] = seg;
    
    seg.type_c = 0;
    seg.dpl = 3;
    gdt_table[3] = seg;
    
    seg.type_c = 1;
    gdt_table[4] = seg;
    
    gdt_ptr->limit = (sizeof(gdt_segment_t) * 5) - 1;
    gdt_ptr->base_low = (uint32_t)dest;
    gdt_ptr->base_high = 0x00000000;
    
    *size = sizeof(gdt_segment_t) * 5;
    
    return 0;
}

/*
 *  we only set up an entry for #GP (13) and #PF (14)
 *  #GP will catch any other exception
 */
uint32_t mem32_build_idt(idt_ptr_t *idt_ptr, void* dest, uint32_t *size, kernel64_isr_ptrs_t *isr_ptrs) {
    if ((uint32_t)dest % 8 != 0)
        return 1;
    
    idt_segment_t *idt_table = (idt_segment_t*)dest;
    
    idt_segment_t idt_zero_entry = {0};
    
    idt_segment_t idt_gp_entry = {
        .offset_low = 0xFFFF & isr_ptrs->gp_low,
        .selector = 0x8,
        .ist = 0,
        .type = 0b10000110,
        .offset_middle = (0xFFFF0000 & isr_ptrs->gp_low) >> 16,
        .offset_high = isr_ptrs->gp_high,
        .zero = 0
    };
    
    idt_segment_t idt_pf_entry = {
        .offset_low = 0xFFFF & isr_ptrs->pf_low,
        .selector = 0x8,
        .ist = 0,
        .type = 0b10000110,
        .offset_middle = (0xFFFF0000 & isr_ptrs->pf_low) >> 16,
        .offset_high = isr_ptrs->pf_high,
        .zero = 0
    };
    
    for (uint32_t i=0; i<13; i++) {
        idt_table[i] = idt_zero_entry;
    }
    
    idt_table[13] = idt_gp_entry;
    idt_table[14] = idt_pf_entry;

    idt_ptr->limit = (sizeof(idt_segment_t) * 14) - 1;
    idt_ptr->base_low = (uint32_t)dest;
    idt_ptr->base_high = 0x00000000;
    
    *size = sizeof(idt_segment_t) * 14;

    return 0;
}

/*
 *  All-in-one routine to create the page tables
 *  needed for a transition into 64-bit mode
 * 
 *  stores the location of the highest-level page table
 *  in the pointer pointed to by page_table_ptr_ptr
 */
 
/* PML4E (512 GB) or PDPTE (1 GB) or PDE (2 MB) or PTE (4KB)
 *   0     - present
 *   1     - r/w
 *   2     - u/s
 *   3     - pwt
 *   4     - pcd
 *   5     - a
 *   6     - PML4E/PDPTE: -, PDE/PTE: d
 *   7     - PML4E/PDPTE/PDE: 0, PTE: pat
 *   8     - PML4E/PDPTE/PDE: -, PTE: g
 *   9:11  - -
 *   12:47 - ptr
 *   48:51 - 0
 *   52:62 - -
 *   63    - xd
 */
 
#define P_ATTR_PRESENT (BIT_0)
#define P_ATTR_READWRITE (BIT_1)
#define P_ATTR_USERMODE (BIT_2)
#define P_ATTR_WTCACHE (BIT_3)
#define P_ATTR_NOCACHE (BIT_4)
#define P_ATTR_ACCESSED (BIT_5)
#define P_ATTR_DIRTY (BIT_6)
#define P_ATTR_PAT (BIT_7)
#define P_ATTR_GLOBAL (BIT_8)

void make_entry(uint32_t *ptr, uint32_t entry, uint32_t addr_low, uint32_t addr_high, uint16_t attr, uint32_t nx) {
    ptr[entry*2] = (addr_high & 0x7FFFFFFF) | (nx ? BIT_31 : 0);
    ptr[entry*2+1] = (addr_low & 0xFFFFF000) | (attr & 0xFFF);
}

uint32_t mem32_setup_early_paging(void **page_table_ptr_ptr, void *kernel64_start, void *kernel64_end, uint32_t verbose) {
    
    uint32_t *pml4_ptr;
    
    //first check if there is a free page at 0x3000 (for the PML4)
    if (mem32_check((uint32_t)*page_table_ptr_ptr, 0x1000)) {
        //put the page tables elsewhere (?)
        return 1;
    } else {
        pml4_ptr = (uint32_t*)(*page_table_ptr_ptr);
    }
    
    *page_table_ptr_ptr = pml4_ptr;
    
    //clear this memory
    for (uint32_t i=0; i<0x400; i++) {
        pml4_ptr[i] = 0;
    }
    
    kterm_write("pml4_ptr = ");
    kterm_write_ui32hx(pml4_ptr);
    kterm_write_line();
    
    //recursive map
    make_entry(pml4_ptr, 511, (uint32_t)pml4_ptr, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    
    //now map kernel64 to high memory
    
    //we're going to put these tables after the kernel
    uint32_t *pt_ptr = ADJUST_PTR(kernel64_end,0x1000);
    
    //determine how many pages this will require:
    
    uint32_t npages = (ADJUST_PTR(kernel64_end,0x1000)-kernel64_start) / 0x1000;
    
    if (verbose) {
        kterm_write("[note] mapping ");
        kterm_write_ui32d(npages);
        kterm_write(" pages of memory\n");
    }
    
    //the top will be all ones (guaranteed to be higher half)
    //this is bits 48:63 (0xFFFF000000000000)
    
    //first add PML4 entry that corresponds to KERNEL64_BASE
    //this is bits 39:47 (0xFF8000000000)
    
    uint32_t p4 = (KERNEL64_BASE & 0xFF8000000000UL) >> 39UL;
    
    //then add PDPTE entry
    //this is bits 30:38 (0x7FC0000000)
    
    uint32_t p3 = (KERNEL64_BASE & 0x7FC0000000) >> 30UL;
    
    //then add the PDE entry
    //this is bits 21:29 (0x3FE00000)
    
    uint32_t p2 = (KERNEL64_BASE & 0x3FE00000) >> 21UL;
    
    //finally add the PTE
    //this is bits 12:20 (0x1FF000)
    
    uint32_t p1 = (KERNEL64_BASE & 0x1FF000) >> 12UL;
    
    //the rest will always be zero (guaranteed to be page aligned)
    //this is bits 0:11 (0xFFF)
    
    if (verbose) {
        kterm_write("[note] p4, p3, p2, p1: ");
        kterm_write_ui32d(p4);
        kterm_write(", ");
        kterm_write_ui32d(p3);
        kterm_write(", ");
        kterm_write_ui32d(p2);
        kterm_write(", ");
        kterm_write_ui32d(p1);
        kterm_write_line();
    }
    
    //TODO: ensure there is available space before we just start writing
    
    make_entry(pml4_ptr, p4, (uint32_t)pt_ptr, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    
    for (uint32_t i=0; i<0x400; i++) {
        pt_ptr[i] = 0;
    }
    
    make_entry(pt_ptr, p3, ((uint32_t)pt_ptr)+0x1000, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    
    //make_entry(pt_ptr, 511, (uint32_t)pt_ptr, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    
    pt_ptr += 0x1000;
    for (uint32_t i=0; i<0x400; i++) {
        pt_ptr[i] = 0;
    }
    
    make_entry(pt_ptr, p2, ((uint32_t)pt_ptr)+0x1000, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    
    //make_entry(pt_ptr, 511, (uint32_t)pt_ptr, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    
    uint32_t *pt_ptr2 = pt_ptr + 0x1000;
    
    uint32_t pd = p2+1;
    uint32_t pmax = 512;
    uint32_t pn = 0;
    
    for (uint32_t i=0; i<0x400; i++) {
        pt_ptr2[i] = 0;
    }
    
    for (uint32_t p=p1; p<npages; p++) {
        if (p == pmax) {
            pd++;
            pmax += 512;
            pt_ptr2 += 0x1000;
            for (uint32_t i=0; i<0x400; i++) {
                pt_ptr2[i] = 0;
            }
            make_entry(pt_ptr, pd, (uint32_t)pt_ptr2, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
            //make_entry(pt_ptr2, 511, (uint32_t)pt_ptr2, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
        }
        if (verbose) {
            kterm_write("[note] mapping ");
            kterm_write_ui32hx(kernel64_start + (p * 0x1000));
            kterm_write(" (slot ");
            kterm_write_ui32d(p % 512);
            kterm_write(")\n");
        }
        make_entry(pt_ptr2, p % 512, (uint32_t)kernel64_start + (pn * 0x1000), 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
        pn++;
    }
    
    //finally identity map the first 0x20000 (2MB)
    pt_ptr2 += 0x1000;
    
    make_entry(pml4_ptr, 0, (uint32_t)pt_ptr2, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    for (uint32_t i=0; i<0x400; i++) {
        pt_ptr2[i] = 0;
    }
    make_entry(pt_ptr2, 0, ((uint32_t)pt_ptr2) + 0x1000, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    pt_ptr2 += 0x1000;
    for (uint32_t i=0; i<0x400; i++) {
        pt_ptr2[i] = 0;
    }
    make_entry(pt_ptr2, 0, ((uint32_t)pt_ptr2) + 0x1000, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    pt_ptr2 += 0x1000;
    for (uint32_t i=0; i<0x200; i++) {
        make_entry(pt_ptr2, i, i*4096, 0, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
    }
    /*
    for (uint32_t i=0x200; i<0x400; i++) {
        pt_ptr2[i] = 0;
    }
    */
    if (verbose) {
        kterm_write("[note] page tables end at ");
        kterm_write_ui32hx(pt_ptr2+0x1000);
        kterm_write(" (");
        kterm_write_ui32d(((uint32_t)pt_ptr2+0x1000-(uint32_t)ADJUST_PTR(kernel64_end,0x1000))/0x1000);
        kterm_write(" tables)\n");
        //kterm_write_line();
    }
    
    return 0;
}


/*  this builds a PML4 table at the specified location
 *  which must have at least 0x1000 bytes free
 *  and sets up self-referencing in entry 511
 */
/*
void make_page_entry(uint32_t *entry, pdp_attr_t attr, uint32_t addr_low, uint32_t addr_high, uint32_t nx) {
    
    addr_high = addr_high & 0x7FFFFFFF | (nx ? BIT_31 : 0);
    addr_low = addr_low & 0xFFFF;
    
    uint32_t entry_low = *(uint8_t*)&attr & addr_low;
    uint32_t entry_high = addr_high;
    
    entry[0] = entry_high;
    entry[1] = entry_low;
} 

uint32_t mem32_build_page_tables(void *dest) {
    if ((uint32_t)dest % 4096 > 0)
        return 1;
    
    uint32_t *ptr = (uint32_t*)dest; 
    
    //clear memory
    for (uint32_t i=0; i<0x400; i++) { //0x400 = 1024
        ptr[i] = 0;
    }
    
    //setup self-referencing table
    pdp_attr_t attr = {.p = 1, .w = 1, .s = 1, .wt = 0, .cd = 0, .a = 0, .d = 0, .ps = 0};
    make_page_entry(&ptr[1022], attr, (uint32_t)ptr, 0, 0);
    
    return 0;
}

uint32_t mem32_map_kernel64(void* page_table, void* address, uint32_t size) {
    //enforce that address lies on page border
    if (address % 4096 > 0)
        return 1;
    
}
*/
/*
uint32_t mem32_map_page(void* page_table, uint32_t virtual_low, uint32_t virtual_high, uint32_t physical_low, uint32_t physical_high, uint32_t size) {
    uint32_t pml4_ptr = (uint32_t)ptr;
    uint32_t pdpt_ptr = (uint32_t*)(dest + 0x1000);
    uint32_t pd_ptr = (uint32_t*)(dest + 0x2000);
    uint32_t pt_ptr = (uint32_t*)(dest + 0x3000);
    
    //pml4 = bits 39:47
    //pdpt = bits 30:38
    //pd   = bits 21:29
    //pt   = bits 12:20
    //raw  = bits 00:11
    
    uint32_t raw_bits = 0b
    uint32_t pml4_bits = 
}
*/
uint32_t mem32_check(uint32_t addr, uint32_t size) {
    for (uint32_t i=0; i<mmap_entry_count; i++) {
        if (mmap[i].address <= addr && addr+size <= mmap[i].address + mmap[i].length) {
            return 0;
        }
    }
    
    return 1;
}

uint32_t mem32_space(uint32_t addr) {
    for (uint32_t i=0; i<mmap_entry_count; i++) {
        if (mmap[i].address <= addr && addr <= mmap[i].address + mmap[i].length) {
            return (mmap[i].length - (addr - mmap[i].address));
        }
    }
    return 0;
}
