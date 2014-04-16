#ifndef _PAGING_H
#define _PAGING_H

#include <stddef.h>
#include <stdint.h>

void map(uint64_t addr, uint64_t page, uint64_t write, uint64_t user, uint64_t nx);
void unmap(uint64_t addr);
void map_query(uint64_t addr, uint64_t *page);
void dump_paging(void);


#endif

