bits 64

%macro save_regs 0
push rax
push rcx
push rdx
push rsi
push rdi
push r8
push r9
push r10
push r11
%endmacro

%macro restore_regs 0
pop r11
pop r10
pop r9
pop r8
pop rdi
pop rsi
pop rdx
pop rcx
pop rax
%endmacro

%define SAVE_SIZE 72

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

extern kernel64_fault_gp
isr_gp_fn:
    cli
    mov rdi, [RSP+0x8]
    call kernel64_fault_gp
    mov rax, 13
.hang:
    hlt
    jmp .hang

extern kernel64_fault_pf
isr_pf_fn:
    cli
    save_regs
    mov rdi, [RSP+0x8+SAVE_SIZE]
    mov rsi, [RSP+SAVE_SIZE]
    mov rdx, cr2
    call kernel64_fault_pf
    cmp rax, 0
    je .noret
    restore_regs
    iretq
.noret:
    mov rax, 14
.hang:
    hlt
    jmp .hang
    
    
;extern kernel64_fault_df
isr_df_fn:
    cli
    mov rax, 8
.hang:
    hlt
    jmp .hang
    
;extern kernel64_fault_i80
isr_i80_fn:
    cli
    mov rax, 80
.hang:
    hlt
    jmp .hang

section .k64isr progbits noexec nowrite noalloc align=8
    db 0xFF,'ISRPTRS'
    dq isr_gp_fn
    dq isr_pf_fn
    dq isr_df_fn
    dq isr_i80_fn
