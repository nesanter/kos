bits 32

MBALIGN     equ 1<<0
MEMINFO     equ 1<<1
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
MAGICCHECK  equ 0x2BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

ERROR_PTR   equ 0xB8002

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    
section .low_mem progbits noalloc exec write
global bootstrap64
align 8
bootstrap64:
    times 64 db 0
    
section .bootstrap_stack progbits noalloc noexec write
align 4
stack_bottom:
     times 16384 db 0
stack_top:

section .data
global kernel32_error_code, kernel32_gdt, kernel32_idt
align 4
kernel32_error_code:
    dw 0x403
kernel32_gdt:
    dw 0x0
    dd 0x0
    dd 0x0
kernel32_idt:
    dw 0x0
    dd 0x0
    dd 0x0

section .text
global _start
global kernel32_hang, kernel32_finalize, read_cr0, read_cr4
_start:

    cmp eax, MAGICCHECK
    jne .no_mboot
    
    mov esp, stack_top
    sub esp, 0x8
    mov [esp], ebx ;save multiboot pointer
    
    extern kernel32_main
    call kernel32_main
    
    jmp .kernel_ret
    
; multiboot sanity check failed
; hang with 'M'
.no_mboot:
    mov ax, 0x44D
    mov [kernel32_error_code], ax
    jmp kernel32_hang

; stage1 kernel returned
; hang with white-on-black
.kernel_ret:
    mov ax, [kernel32_error_code]
    and ax, 0x00FF
    or ax, 0x0F00
    mov [kernel32_error_code], ax
    jmp kernel32_hang

kernel32_hang:
    mov ax, [kernel32_error_code]
    mov edx, dword ERROR_PTR
    mov [edx], ax
    cli
.hang:
    hlt
    jmp .hang

kernel32_finalize:

    ;jmp kernel32_hang

    mov esi, [esp+0x8] ;cr3_t
    ;mov ebx, [esp+0x8] ;cr4_t
    ;mov edx, [esp+0xC] ;cr0_t
    mov edi, [esp+0x4] ;kernel_handoff_t*
    ;mov edx, [esp+0x10] ;void* (bootstrap64_low)
    ;mov edx, edi
    ;add edx, 0x8
    ;mov bx, [edx]
    ;mov [kernel32_gdt], bx
    ;mov ebx, [edx+0x2]
    ;mov [kernel32_gdt+2], ebx
    ;mov ebx, [edx+0x6]
    ;mov [kernel32_gdt+6], ebx
    ;mov edx, edi
    ;add edx, 0x1B
    ;mov bx, [edx]
    ;mov [kernel32_idt], bx
    ;mov ebx, [edx+0x2]
    ;mov [kernel32_idt+2], ebx
    ;mov ebx, [edx+0x6]
    ;mov [kernel32_idt+6], ebx
    
    ;enable PAE bit
    mov ebx, cr4
    or ebx, 1 << 5 ;pae
    mov cr4, ebx
    
    ;put page table in cr3
    mov cr3, esi
    
    ;set LME bit in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ;enable PG bit (and others)
    mov edx, cr0
    or edx, 1 << 31 ;pg
    or edx, 1 << 0 ;pe
    or edx, 1 << 2 ;set em
    xor edx, 1 << 2 ;disable em
    or edx, 1 << 5 ;ne
    mov cr0, edx
    
    ;mov edx, edi
    ;add edx, 8
    lgdt [kernel32_gdt]
    
    ;add edx, 10
    lidt [kernel32_idt]
    
    ;not really sure whether this is needed?
    mov dx, 0x10 ; (GDT[2] is ring 0 data)
    mov fs, dx
    mov gs, dx
    
    jmp 0x8:bootstrap64
    
    ;sub esp, 0x6
    ;mov [esp], eax
    ;mov dx, 0x8 ; (GDT[1] is ring 0 code)
    ;mov [esp+0x4], dx
    
    ;far return to spoofed location
    ;retf
    ;this doesn't work because the entry to kernel64 is a 64-bit pointer
    ;and we can only jump to lower-half addresses
