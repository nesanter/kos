#ifndef __MODE_KERNEL32
#error "THIS HEADER IS STAGE1 ONLY!"
#endif

#ifndef _CPUID32_H
#define _CPUID32_H

#include <stddef.h>
#include <stdint.h>
/*
 *  cpuid_precheck_info_t kernel32_cpuid_precheck(): check if cpuid is available [defined in cpuid.asm]
 *  void kernel32_cpuid(uint32_t eax, cpuid_info_t *info): cpuid wrapper [defined in cpuid.asm]
 *  void kernel32_enable_sse(): turn on sse [defined in cpuid.asm]
 */

typedef struct _cpuid_info {
    uint32_t eax, ebx, ecx, edx;
} cpuid_info_t;

#define CPUID_VID_INTEL 1
#define CPUID_VID_AMD 2
#define CPUID_VID_UNKNOWN 255

typedef struct _cpuid_precheck_info {
    uint8_t valid, max_low, max_high, vendor;
} cpuid_precheck_info_t;

cpuid_precheck_info_t* kernel32_cpuid_precheck();
void kernel32_cpuid(uint32_t eax, cpuid_info_t *info);

/* CRn breakdown */

typedef struct _cr0 {
    uint32_t pe:1; //1 = protected mode [set by GRUB]                                       0
    uint32_t mp:1; //1 = fault on WAIT if TS [set by k32]                                   1
    uint32_t em:1; //1 = no fpu/sse hardware support [cleared by k32]                       2
    uint32_t ts:1; //1 = task switch ocurred [set by cpu; leave alone]                      3
    uint32_t et:1; //hardcoded to 1                                                         4
    uint32_t ne:1; //1 = new-style x87 errors [set by k32]                                  5
    uint32_t _res0:10; //                                                                   6:15
    uint32_t wp:1; //1 = supervisor cannot access RO pages [modified by k64; leave alone]   16
    uint32_t _res1:1; //                                                                    17
    uint32_t am:1; //1 = alignment checking in protected/virtual-8086 [leave alone]         18
    uint32_t _res:10; //                                                                    19:28
    uint32_t nw:1; //1 = write-back caching [leave alone]                                   29
    uint32_t cd:1; //1 = cache disabled [leave alone]                                       30
    uint32_t pg:1; //1 = paging enabled [set as part of handoff by k32]                     31
} cr0_t;

// cr1 is reserved

// cr2 contains page-fault linear address

// cr3 contains physical address of base of page table hierarchy plus two unused flags

typedef struct _cr4 {
    uint32_t vme:1; //1 = virtual-8086 extensions on [leave alone]
    uint32_t pvi:1; //1 = protected-mode virtual interrupts on [leave alone]
    uint32_t tsd:1; //1 = RDTSC/RDTSCP is a privileged instruction [modified by k64; leave alone]
    uint32_t de:1;  //1 = use of DR4/DR5 is undefined [???]
    uint32_t pse:1; //1 = 4MB pages in 32-bit paging mode [leave alone]
    uint32_t pae:1; //1 = enable PAE [set as part of handoff by k32]
    uint32_t mce:1; //1 = enable machine-check exceptions [modifed by k64; leave alone]
    uint32_t pge:1; //1 = global paging enabled [modified by k64; leave alone]
    uint32_t pce:1; //1 = RDPMC is non privileged [modified by k64; leave alone]
    uint32_t osfxsr:1; //1 = say we support SSE context switch [set by k32]
    uint32_t osxmmexcpt:1; //1 = say we support SSE exceptions [set by k32]
    uint32_t _res0:2;
    uint32_t vmxe:1; //1 = vmx is enabled [leave alone]
    uint32_t smxe:1; //1 = smx is enabled [leave alone]
    uint32_t _res1:1;
    uint32_t fsgsbase:1; //1 = enable the FSGS instructions [leave alone]
    uint32_t pcide:1; //1 = process-context indentifiers enabled [handled by k64; leave alone]
    uint32_t osxsave:1; //1 = XSAVE and processor extended states are enabled [???]
    uint32_t _res2:1;
    uint32_t smep:1; //supervisor-mode execution prevention [leave alone]
    uint32_t _res3:11;
} cr4_t;

//cr8 is only accessible in 64-bit mode (bits 0:3 control task priority level masking)

//xcr0 is only accessible in 64-bit mode
/*
typedef struct _xcr0 {
    uint32_t x87:1; //hardcoded to 1
    uint32_t sse:1; //1 = XSAVE/etc. manages XMMn [set by k64]
    uint32_t avx:1; //1 = AVX enabled and XSAVE/etc. manages YMMn [set by k64]
    uint32_t 
} xcr0_t;
*/

extern cr0_t read_cr0();
extern cr4_t read_cr4();

#endif
