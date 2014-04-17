#include "moddefs.h"
#include "context.h"
#include "early_kterm.h"

typedef struct _mod_context {
    mod_init_return_t init_info;
    uint64_t cr3; //pml4 base + pcid
    context_t context;
} mod_context_t;

typedef struct _mod_context_block {
    mod_context_t contexts[8];
    uint64_t ncontexts;
    struct _mod_context_block *next;
} mod_context_block_t;

uint64_t module_init(void) {
    ekterm_write_hex(sizeof(mod_context_t),16);
    return 0;
}

uint64_t module_load(uint64_t base) {
    return 0;
}
