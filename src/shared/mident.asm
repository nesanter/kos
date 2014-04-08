bits 64

;__PREPME
;__OUTNAME:mident_p.asm

section .modinfo progbits noexec nowrite noalloc
    db 255,"MIDENT",0
    db "__MODNAME",0

