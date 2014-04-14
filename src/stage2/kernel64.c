#include "handoff.h"
#include "early_kterm.h"
#include "palloc.h"

void kernel64_main(uint32_t handoff_ptr_raw) {
    ekterm_initialize();
    
    kernel_handoff_t *handoff_ptr = (kernel_handoff_t*)(((uint64_t)handoff_ptr_raw) + 0xFFFF800000000000);
    
    if (handoff_ptr->magic[0] != 0x534E524B || handoff_ptr->magic[1] != 0x4C455454) {
        ekterm_write("[error] bad handoff magic received\n");
        return;
    }
    
    ekterm_write_hex(handoff_ptr->reserved[0].address,16);
    ekterm_write_char('\n');
    ekterm_write_hex(handoff_ptr->reserved[0].length,16);
    ekterm_write_char('\n');
    
    void* early_safe = (void*)(handoff_ptr->reserved[0].address + handoff_ptr->reserved[0].length);
    
    ekterm_write_hex((uint64_t)early_safe,16);
    ekterm_write_char('\n');
    
    uint64_t *p = (uint64_t*)0x500000;
    *p = 3;
    
    return;
    
    //ekterm_write_hex(sizeof(kernel_handoff_t),16);
    //ekterm_write_char('\n');
    
    if (palloc_init(early_safe, handoff_ptr->mmap, handoff_ptr->mmap_entry_num,
                    handoff_ptr->reserved, handoff_ptr->reserved_entry_num)) {
                        
        ekterm_write("oops during palloc_init");
        return;
    }
    
    pdump();
    
    
    ekterm_write("hello, world!");
}

void kernel64_fault_gp(uint64_t ip) {
    ekterm_write("\n#GP occurred @ ");
    ekterm_write_hex(ip, 16);
}

uint64_t kernel64_fault_pf(uint64_t ip,uint64_t errc, uint64_t page) {
    ekterm_write("\n#PF occurred @ ");
    ekterm_write_hex(ip, 16);
    ekterm_write(" for page @ ");
    ekterm_write_hex(page, 16);
    //return 1 to resume
    //return 0 to halt
    return 0;
}
