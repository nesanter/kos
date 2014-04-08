/*  
 *  linker32.ld is used for linking stage1
 *  linker64.ld is used for linking stage2
 *  linker_m.ld is used for linking modules
 */ 

ENTRY(_start)

K64_BASE = 0xFFFFA00000000000;

SECTIONS
{
    . = K64_BASE;
    
    .text BLOCK(4096) : AT(ADDR(.text) - K64_BASE)
    {
        *(.text)
        *(.rodata*)
        *(.eh_frame*)
    }
    
    .data BLOCK(4096) : AT(ADDR(.data) - K64_BASE)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - K64_BASE)
    {
        *(.bss)
        *(COMMON*)
    }
    
    .modinfo : AT(ADDR(.modinfo) - K64_BASE)
    {
        *(.modinfo)
    }
    
    /DISCARD/ :
    {
        *(.comment)
    }
}
