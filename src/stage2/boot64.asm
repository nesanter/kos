bits 64

section .data
my_data:
    db "i love mai snrk"

section .bss
my_bss:
    resb 1

section .text
global _start
_start:
    ;edi contains a pointer to the handoff struct
    extern kernel64_main
    call kernel64_main

    cli
.hang:
    hlt
    jmp .hang

isr_gp_fn:
    iretq

isr_pf_fn:
    iretq
    
section .k64isr progbits noexec nowrite noalloc align=8
    db 0xFF,'ISRPTRS'
    dq isr_gp_fn
    dq isr_pf_fn
