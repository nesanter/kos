#include "handoff.h"


void kernel64_main(uint32_t handoff_ptr) {
    volatile uint16_t *out = (uint16_t*)0xB8000;
    
    out[1] = 0x401;
}
