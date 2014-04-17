#ifndef _KMALLOC_H
#define _KMALLOC_H

#include <stddef.h>
#include <stdint.h>

void* kmalloc(uint64_t sz);
void kfree(void *ptr);

#endif
