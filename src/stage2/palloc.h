#ifndef PALLOC_H
#define PALLOC_H

#include <stddef.h>
#include <stdint.h>

uint64_t palloc_init(void *dest, mmap_entry_t *mmap, uint64_t mmap_entries,
                     mmap_entry_t *reserved, uint64_t reserved_entries);
uint64_t palloc(void **ptrs, uint64_t num_pages);
uint64_t pfree(void **ptrs, uint64_t num_pages);
uint64_t preserve(void *ptr);

#endif
