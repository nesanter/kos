bits 32

;VID_INTEL_EBX       equ 0x47656E75
;VID_INTEL_EDX       equ 0x696E6549
;VID_INTEL_ECX       equ 0x6E74656C
VID_INTEL_EBX       equ 0x756E6547
VID_INTEL_EDX       equ 0x49656E69
VID_INTEL_ECX       equ 0x6C65746E
VID_INTEL_N         equ 1

;VID_AMD_EBX         equ 0x41757468
;VID_AMD_EDX         equ 0x656E7469
;VID_AMD_ECX         equ 0x63414D44
VID_AMD_EBX         equ 0x68747541
VID_AMD_EDX         equ 0x69746E65
VID_AMD_ECX         equ 0x444D4163
VID_AMD_N           equ 2

VID_UNKNOWN_N       equ 255

section .data
align 4
cpuid_valid:
    db 0, 0, 0, 0

section .text
global kernel32_cpuid_precheck, kernel32_cpuid, kernel32_enable_sse
global read_cr0, read_cr4
kernel32_cpuid_precheck:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    shr eax, 21
    and eax, 1
    push ecx
    popfd
    mov [cpuid_valid], al
    test eax, eax
    jz .done
    mov eax, 0
    cpuid
    mov [cpuid_valid+0x1], al
    call identify_vendor
    mov eax, 0x80000000
    cpuid
    mov [cpuid_valid+0x2], al
.done:
    mov eax, cpuid_valid
    ret
    
kernel32_cpuid:
    push edi
    push ebx
    mov eax, [esp+0x0C]
    mov edi, [esp+0x10]
    cpuid
    mov [edi], eax
    mov [edi+0x4], ebx
    mov [edi+0x8], ecx
    mov [edi+0xC], edx
    pop ebx
    pop edi
    ret

identify_vendor:
    cmp ebx, VID_INTEL_EBX
    je .intel
    cmp ebx, VID_AMD_EBX
    je .amd
.unknown:
    mov al, VID_UNKNOWN_N
    mov [cpuid_valid+0x3], al
    ret
.intel:
    cmp edx, VID_INTEL_EDX
    jne .unknown
    cmp ecx, VID_INTEL_ECX
    jne .unknown
    mov al, VID_INTEL_N
    mov [cpuid_valid+0x3], al
    ret
.amd:
    cmp edx, VID_AMD_EDX
    jne .unknown
    cmp ecx, VID_AMD_ECX
    jne .unknown
    mov al, VID_AMD_N
    mov [cpuid_valid+0x3], al
    ret

read_cr0:
    mov eax, cr0
    ret

read_cr4:
    mov eax, cr4
    ret
