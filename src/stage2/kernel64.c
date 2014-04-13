#include "handoff.h"
#include "early_kterm.h"

void kernel64_main(uint32_t handoff_ptr) {
    ekterm_initialize();
    
    ekterm_write("hello, world!");
}
