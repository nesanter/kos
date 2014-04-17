#ifndef _LOADER_H
#define _LOADER_H

#include <stddef.h>
#include <stdint.h>

uint64_t module_init(void);
uint64_t module_load(uint64_t base);

#endif
