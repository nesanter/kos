#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <stddef.h>
#include <stdint.h>

typedef struct _context {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, r8, r9, r10, r11, r12;
    uint64_t xmm0, ymm0, xmm1, ymm1, xmm2, ymm2, xmm3, ymm3,
             xmm4, ymm4, xmm5, ymm5, xmm6, ymm6, xmm7, ymm7,
             xmm8, ymm8, xmm9, ymm9, xmm10, ymm10, xmm11, ymm11,
             xmm12, ymm12, xmm13, ymm13, xmm14, ymm14, xmm15, ymm15;
    uint64_t rsp, rbp, rflags;
} context_t;

#endif
