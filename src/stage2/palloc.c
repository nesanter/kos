#include "handoff.h"
#include "palloc.h"
#include "early_kterm.h"

typedef struct _pmap {
    void *start;
    void *end;
    struct _pmap *next;
} pmap_t;

pmap_t *palloc_map; //linked list
uint64_t palloc_map_size;
#define PALLOC_MAP_MAX_SIZE (24576)

uint64_t palloc_init(void *dest, mmap_entry_t *mmap, uint32_t mmap_entries,
                 mmap_entry_t *reserved, uint32_t reserved_entries) {
    
    palloc_map = (pmap_t*)dest;
    
    ekterm_write("mmapn ");
    ekterm_write_hex(mmap_entries,16);
    ekterm_write("\nresn ");
    ekterm_write_hex(reserved_entries,16);
    ekterm_write_char('\n');
    
    for (uint64_t i=0; i<mmap_entries; i++) {
        palloc_map[i].start = (void*)mmap[i].address;
        palloc_map[i].end = (void*)mmap[i].address + mmap[i].length;
        if (i>0) {
            palloc_map[i-1].next = &palloc_map[i];
        }
        palloc_map_size += sizeof(pmap_t);
    }
    palloc_map[mmap_entries-1].next = NULL;
    
    pdump();
    
    for (uint64_t i=0; i<reserved_entries; i++) {
        for (uint64_t j=0; j<reserved[i].length; j+=0x1000) {
            preserve((void*)reserved[i].address+j);
        }
    }
    
    for (uint64_t i=0; i<PALLOC_MAP_MAX_SIZE; i+=0x1000) {
        preserve(((void*)palloc_map)+i);
    }
    
    return 0;
}

uint64_t palloc(void **ptrs, uint64_t num_pages) {
    
    uint64_t i=0, n;
    
    pmap_t *p = palloc_map;
    
    if (!p)
        return 1;
    
    while (num_pages) {
        n = (p->end - p->start) / 0x1000;
        if (n > num_pages) {
            n = num_pages;
        }
        num_pages -= n;
        while (n--) {
            ekterm_write("palloc: ");
            ekterm_write_hex((uint64_t)p->start,16);
            ekterm_write_char('\n');
            ptrs[i++] = p->start;
            p->start += 0x1000;
        }
        p = p->next;
        if (!p) {
            return 1;
        }
    }
    
    return 0;
    
}

//if we looped through the whole map for each
//pointer this would be rather painful
//instead we're going to try and optimize
//by hoping that consecutive ptrs
//refer to consecutive pages
uint64_t pfree(void **ptrs, uint64_t num_pages) {
    
    //start by forming blocks from ptrs
    //once a block cannot be expanded
    //insert it into the general map
    
    uint64_t i=0;
    
    while (i<num_pages) {
    
        pmap_t b = {.start = ptrs[i], .end = ptrs[i]+0x1000};
    
        for (i++; i<num_pages; i++) {
            if (b.start - ptrs[i] == 0x1000) {
                b.start -= 0x1000;
            } else if (ptrs[i] - b.end == 0x1000) {
                b.end += 0x1000;
            } else {
                break;
            }
        }
        
        pmap_t *p = palloc_map;
        pmap_t *prev = NULL;
        
        while (p) {
            
            if (p->start == b.end) {
                //place before
                p->start = b.start;
                break;
            }
            if (p->end == b.start) {
                //place after
                p->end = b.end;
                //check for merge
                if (p->next) {
                    if (p->next->start <= p->end) {
                        p->end = p->next->end;
                        p->next = p->next->next;
                        if (!p->next) {
                            //block removed was last
                            palloc_map_size -= sizeof(pmap_t);
                        }
                    }
                }
                break;
            }
            
            prev = p;
            p = p->next;
        }
        
        if (!p) {
            //place at end
            pmap_t *newp = (pmap_t*)((void*)palloc_map + palloc_map_size);
            
            palloc_map_size += sizeof(pmap_t);
                
            if (palloc_map_size > PALLOC_MAP_MAX_SIZE) {
                return -1;
            }
            
            *newp = b;
            
            if (prev) {
                prev->next = newp;
            } else {
                palloc_map = newp;
            }
        }
        
    }
    
    return 0;
    
}

uint64_t preserve(void *ptr) {
    
    pmap_t *p = palloc_map;
    pmap_t *prev = NULL;
    
    //ekterm_write("hello, snrk!");
    
    //ekterm_write_hex((uint64_t)p,16);
    //return 0;
    
    while (p) {
        //ekterm_write_hex((uint64_t)p,16);
        //ekterm_write_char('\n');
        if (p->start <= ptr && ptr < p->end) {
            if (ptr - p->start <= 0x1000) {
                //take from front
                //ekterm_write("front\n");
                p->start += 0x1000;
                if (p->end - p->start < 0x1000) {
                    //smaller than page,
                    //so remove
                    ekterm_write("removing\n");
                    if (prev) {
                        prev->next = p->next;
                    } else {
                        palloc_map = p->next;
                    }
                }
                return 0;
            } else if (p->end - ptr <= 0x1000) {
                //take from end
                ekterm_write("end\n");
                p->end -= 0x1000;
                if (p->end - p->start < 0x1000) {
                    //smaller than page,
                    //so remove
                    if (prev) {
                        prev->next = p->next;
                    } else {
                        palloc_map = p->next;
                    }
                }
                return 0;
            } else {
                //split block in two
                ekterm_write("split\n");
                pmap_t *newp = (pmap_t*)((void*)palloc_map + palloc_map_size);
                
                palloc_map_size += sizeof(pmap_t);
                
                if (palloc_map_size > PALLOC_MAP_MAX_SIZE) {
                    return -1;
                }
                
                newp->end = p->end;
                p->end = (void*)((uint64_t)ptr & 0xFFFFFFFFFFFFF000);
                newp->start = p->end + 0x1000;
                
                newp->next = p->next;
                p->next = newp;
                //this will never result in a block smaller
                //than a page
                return 0;
            }
        }
        prev = p;
        p = p->next;
    }
    
    //no block contains ptr
    //it must be either invalid or reserved already
    return 1;
}

void pdump(void) {
    ekterm_write("pdump (\n");
    pmap_t *p = palloc_map;
    while (p) {
        ekterm_write_hex((uint64_t)p->start, 16);
        ekterm_write(" -> ");
        ekterm_write_hex((uint64_t)p->end, 16);
        ekterm_write_char('\n');
        p = p->next;
    }
    ekterm_write(")\n");
}
