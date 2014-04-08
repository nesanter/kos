/*  
 *  linker32.ld is used for linking stage1
 *  linker64.ld is used for linking stage2
 *  linker_m.ld is used for linking modules
 */ 

ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    .low_mem :
    {
        *(.low_mem)
        *(.multiboot)
    }
    
    .text BLOCK(4096) :
    {
        *(.text)
        *(.rodata*)
    }
    
    .data BLOCK(4096) :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
        *(COMMON*)
    }
    
    .bootstrap_stack BLOCK(4096) :
    {
        *(.bootstrap_stack)
    }
    
    _end = .;
    
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
}
