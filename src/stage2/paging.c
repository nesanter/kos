#include "early_kterm.h"
#include "palloc.h"
#include "utilbits.h"

#define P_ATTR_PRESENT (BIT_0)
#define P_ATTR_READWRITE (BIT_1)
#define P_ATTR_USERMODE (BIT_2)
#define P_ATTR_WTCACHE (BIT_3)
#define P_ATTR_NOCACHE (BIT_4)
#define P_ATTR_ACCESSED (BIT_5)
#define P_ATTR_DIRTY (BIT_6)
#define P_ATTR_PAT (BIT_7)
#define P_ATTR_GLOBAL (BIT_8)

#define BIT_31 0x80000000

uint64_t make_entry(uint64_t addr, uint16_t attr, uint32_t nx) {
    return (addr & 0x7FFFFFFF00000000) | (nx ? 0x8000000000000000UL : 0) | (addr & 0xFFFFF000) | (attr & 0xFFF);
}

/* recursive mapping notes
 *  - pml4: 0xFFFFFFFFFFFFF000 | pml4e
 *  - pdpt: 0xFFFFFFFFFFE00000 | (pml4e << 12) | pdpte
 *  - pd:   0xFFFFFFFFC0000000 | (pml4e << 21) | (pdpte << 12) | pde
 *  - pt:   0xFFFFFF8000000000 | (pml4e << 30) | (pdpte << 21) | (pde << 12) | pte
 */

void map(uint64_t addr, uint64_t page, uint64_t write, uint64_t user, uint64_t nx) {
    
    uint64_t pml4e_bits = (addr & 0x0000FF8000000000UL) >> 39;
    uint64_t pdpte_bits = (addr & 0x0000007FC0000000UL) >> 30;
    uint64_t pde_bits   = (addr & 0x000000003FE00000UL) >> 21;
    uint64_t pte_bits   = (addr & 0x00000000001FF000UL) >> 12;
    
    uint64_t *pml4_ptr = (uint64_t*)(0xFFFFFFFFFFFFF000 | (pml4e_bits << 3));
    uint64_t *pdpt_ptr = (uint64_t*)(0xFFFFFFFFFFE00000 | (pml4e_bits << 12) | (pdpte_bits << 3));
    uint64_t *pd_ptr   = (uint64_t*)(0xFFFFFFFFC0000000 | (pml4e_bits << 21) | (pdpte_bits << 12) | (pde_bits << 3));
    uint64_t *pt_ptr   = (uint64_t*)(0xFFFFFF8000000000 | (pml4e_bits << 30) | (pdpte_bits << 21) | (pde_bits << 12) | (pte_bits << 3));
    
    /*
    ekterm_write_hex((uint64_t)pml4_ptr,16);
    ekterm_write_char('\n');
    ekterm_write_hex((uint64_t)pdpt_ptr,16);
    ekterm_write_char('\n');
    ekterm_write_hex((uint64_t)pd_ptr,16);
    ekterm_write_char('\n');
    ekterm_write_hex((uint64_t)pt_ptr,16);
    ekterm_write_char('\n');
    */
    
    void* new_table;
    if ((*pml4_ptr & 1) == 0) {
        ekterm_write("mapping pml4e\n");
        //we need to allocate a new page for the table
        palloc(&new_table, 1);
        *pml4_ptr = make_entry((uint64_t)new_table, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
        //clear new table
        for (uint64_t i=0; i<0x200; i++) {
            ((uint64_t*)((uint64_t)pdpt_ptr & 0xFFFFFFFFFFFFF000))[i] = 0;
        }
    }
    if ((*pdpt_ptr & 1) == 0) {
        ekterm_write("mapping pdpte\n");
        //we need to allocate a new page for the table
        palloc(&new_table, 1);
        *pdpt_ptr = make_entry((uint64_t)new_table, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
        //clear new table
        for (uint64_t i=0; i<0x200; i++) {
            ((uint64_t*)((uint64_t)pd_ptr & 0xFFFFFFFFFFFFF000))[i] = 0;
        }
    }
    
    if ((*pd_ptr & 1) == 0) {
        ekterm_write("mapping pde\n");
        //we need to allocate a new page for the table
        palloc(&new_table, 1);
        *pd_ptr = make_entry((uint64_t)new_table, P_ATTR_PRESENT | P_ATTR_READWRITE, 0);
        //clear new table
        for (uint64_t i=0; i<0x200; i++) {
            ((uint64_t*)((uint64_t)pt_ptr & 0xFFFFFFFFFFFFF000))[i] = 0;
        }
    }
    *pt_ptr = make_entry(page, P_ATTR_PRESENT | (write ? P_ATTR_READWRITE : 0) | (user ? P_ATTR_USERMODE : 0), nx);
}

void unmap(uint64_t addr) {
    uint64_t pml4e_bits = (addr & 0x0000FF8000000000UL) >> 39;
    uint64_t pdpte_bits = (addr & 0x0000007FC0000000UL) >> 30;
    uint64_t pde_bits   = (addr & 0x000000003FE00000UL) >> 21;
    uint64_t pte_bits   = (addr & 0x00000000001FF000UL) >> 12;
    
    uint64_t *pt_ptr   = (uint64_t*)(0xFFFFFF8000000000 | (pml4e_bits << 30) | (pdpte_bits << 21) | (pde_bits << 12) | (pte_bits << 3));
    
    *pt_ptr = 0;
    
    asm volatile("invlpg (%0)"::"r"(addr):);
}

uint64_t map_query(uint64_t addr, uint64_t *page) {
    uint64_t pml4e_bits = (addr & 0x0000FF8000000000UL) >> 39;
    uint64_t pdpte_bits = (addr & 0x0000007FC0000000UL) >> 30;
    uint64_t pde_bits   = (addr & 0x000000003FE00000UL) >> 21;
    uint64_t pte_bits   = (addr & 0x00000000001FF000UL) >> 12;
    
    uint64_t *pml4_ptr = (uint64_t*)(0xFFFFFFFFFFFFF000 | (pml4e_bits << 3));
    uint64_t *pdpt_ptr = (uint64_t*)(0xFFFFFFFFFFE00000 | (pml4e_bits << 12) | (pdpte_bits << 3));
    uint64_t *pd_ptr   = (uint64_t*)(0xFFFFFFFFC0000000 | (pml4e_bits << 21) | (pdpte_bits << 12) | (pde_bits << 3));
    uint64_t *pt_ptr   = (uint64_t*)(0xFFFFFF8000000000 | (pml4e_bits << 30) | (pdpte_bits << 21) | (pde_bits << 12) | (pte_bits << 3));
    
    if (!(*pml4_ptr & 1) || !(*pdpt_ptr & 1) || !(*pd_ptr & 1) || (*pt_ptr & 1)) {
        return 0;
    }
    
    if (page) {
        *page = *pt_ptr & 0x7FFFFFFFFFFFF000;
    }
    
    return 1;
}

void dump_paging(void) {
    uint64_t *pml4_ptr = (uint64_t*)(0xFFFFFFFFFFFFF000);
    for (uint64_t i=0; i<0x200; i++) {
        if (pml4_ptr[i] & 1) {
            //entry present
            ekterm_write("pml4[ ");
            ekterm_write_hex(i,3);
            ekterm_write(" ]\n");
            uint64_t *pdpt_ptr = (uint64_t*)(0xFFFFFFFFFFE00000 | (i << 12));
            for (uint64_t ii=0; ii<0x200; ii++) {
                if (pdpt_ptr[ii] & 1) {
                    //entry present
                    ekterm_write("  pdpt[ ");
                    ekterm_write_hex(ii,3);
                    ekterm_write(" ]\n");
                    uint64_t *pd_ptr = (uint64_t*)(0xFFFFFFFFC0000000 | (i << 21) | (ii << 12));
                    for (uint64_t iii=0; iii<0x200; iii++) {
                        if (pd_ptr[iii] & 1) {
                            //entry present
                            ekterm_write("    pd[ ");
                            ekterm_write_hex(iii,3);
                            ekterm_write(" ]\n");
                            uint64_t *pt_ptr = (uint64_t*)(0xFFFFFF8000000000 | (i << 30) | (ii << 21) | (iii << 12));
                            ekterm_write("      pt[ ");
                            uint64_t streak_start = 0;
                            for (uint64_t iiii=0; iiii<0x200; iiii++) {
                                if (pt_ptr[iiii] & 1) {
                                    if (streak_start == 0)
                                        streak_start = iiii+1;
                                    //ekterm_write_hex(iiii,3);
                                    //ekterm_write_char(' ');
                                    //page present
                                    /*
                                    ekterm_write("      pt[");
                                    ekterm_write_hex(iiii,3);
                                    ekterm_write("] -> ");
                                    ekterm_write_hex(pt_ptr[iiii] & 0x7FFFFFFFFFFFF000,16);
                                    ekterm_write_char('\n');
                                    volatile uint64_t j = 10000000;
                                    while (j--);
                                    */
                                } else {
                                    if (streak_start != 0) {
                                        if (streak_start != iiii) { 
                                            ekterm_write_hex(streak_start-1,3);
                                            ekterm_write("-");
                                        }
                                        ekterm_write_hex(iiii-1,3);
                                        ekterm_write(" ");
                                        streak_start = 0;
                                    }
                                }
                            }
                            if (streak_start != 0) {
                                if (streak_start != 0x200) {
                                    ekterm_write_hex(streak_start,3);
                                    ekterm_write("-");
                                }
                                ekterm_write_hex(0x1ff,3);
                                ekterm_write(" ");
                            }
                            ekterm_write("]\n");
                        }
                    }
                }
            }
        }
    }
    
    ekterm_write("[done]");
}

