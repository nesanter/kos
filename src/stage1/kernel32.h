#if defined(__linux__)
#error "CROSS-COMPILER SANITY CHECK FAILED!"
#endif

#ifndef __MODE_KERNEL32
#error "THIS HEADER IS STAGE1 ONLY!"
#endif

#ifndef _KERNEL32_H
#define _KERNEL32_H

#include <stddef.h>
#include <stdint.h>

#include "handoff.h"

#define DEFAULT_KERNEL64 "k64"

#define KERNEL64_BASE (0xFFFF800000000000UL)

/*  
 *  void kernel32_hang(): hangs the kernel [defined in boot.asm]
 *  void kernel32_finalize(): jumps to stage2 [defined in boot.asm]
 *  uint16_t kernel32_error_code: the magic word displayed on a hang [defined in boot.asm]
 */
extern void kernel32_hang();
extern uint16_t kernel32_error_code;
extern uint32_t* bootstrap64;

/*
 *  kernel32_cmdline_t: information extracted from multiboot cmdline
 */

typedef struct _kernel32_cmdline {
    char kernel64_name[32];
    kernel_cmdline_flags_t flags; //defined in handoff.h
} kernel32_cmdline_t;

//void* kernel32_bootstrap64_ptr;

#endif
