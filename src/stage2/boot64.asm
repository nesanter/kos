bits 64

section .text
global _start
_start:

    ;mov rax, 0xFFFF800000000000
    ;mov rbx, esi
    ;or rax, rbx
    ;mov rsp, rax

    ;sti

    ;edi contains a pointer to the handoff struct
    extern kernel64_main
    call kernel64_main

    cli
.hang:
    hlt
    jmp .hang

extern kernel64_fault_gp
isr_gp_fn:
    cli
    mov rdi, [RSP+0x8]
    call kernel64_fault_gp
.hang:
    hlt
    jmp .hang

extern kernel64_fault_pf
isr_pf_fn:
    cli
    mov rdi, [RSP+0x8]
    mov rsi, [RSP]
    mov rdx, cr2
    call kernel64_fault_pf
    ;cmp rax, 0
    ;je .hang
    ;iretq
.hang:
    hlt
    jmp .hang

section .k64isr progbits noexec nowrite noalloc align=8
    db 0xFF,'ISRPTRS'
    dq isr_gp_fn
    dq isr_pf_fn
