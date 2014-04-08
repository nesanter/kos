#ifndef _ELF_H
#define _ELF_H

#include <stddef.h>
#include <stdint.h>

/* 
 *  ELF64 structs
 */
#define ELF64_MAGIC 0x464C457F

#define ELF64_EV_CURRENT 1

//ei_class
#define ELF64_ELFCLASS32 1
#define ELF64_ELFCLASS64 2

//ei_data
#define ELF64_ELFDATA2LSB 1
#define ELF64_ELFDATA2MSB 2

//ei_osabi
#define ELF64_ELFOSABI_SYSV 0
#define ELF64_ELFOSABI_HPUX 1
#define ELF64_ELFOSABI_STANDALONE 255

//e_type
#define ELF64_ET_NONE 0
#define ELF64_ET_REL 1
#define ELF64_ET_EXEC 2
#define ELF64_ET_DYN 3
#define ELF64_ET_CORE 4
#define ELF64_ET_LOOS 0xFE00
#define ELF64_ET_HIOS 0xFEFF
#define ELF64_ET_LOPROC 0xFF00
#define ELF64_ET_HIPROC 0xFFFF

//sh_type
#define ELF64_SHT_NULL 0
#define ELF64_SHT_PROGBITS 1
#define ELF64_SHT_SYMTAB 2
#define ELF64_SHT_STRTAB 3
#define ELF64_SHT_RELA 4
#define ELF64_SHT_HASH 5
#define ELF64_SHT_DYNAMIC 6
#define ELF64_SHT_NOTE 7
#define ELF64_SHT_NOBITS 8
#define ELF64_SHT_REL 9
#define ELF64_SHT_SHLIB 10
#define ELF64_SHT_DYNSYM 11
#define ELF64_SHT_LOOS 0x60000000
#define ELF64_SHT_HIOS 0x6FFFFFFF
#define ELF64_SHT_LOPROC 0x70000000
#define ELF64_SHT_HIPROC 0x7FFFFFFF

//p_type
#define ELF64_PT_NULL 0
#define ELF64_PT_LOAD 1
#define ELF64_PT_DYNAMIC 2
#define ELF64_PT_INTERP 3
#define ELF64_PT_NOTE 4
#define ELF64_PT_SHLIB 5
#define ELF64_PT_PHDR 6

typedef uint64_t elf64_addr_t;
typedef uint64_t elf64_off_t;
typedef uint16_t elf64_half_t;
typedef uint32_t elf64_word_t;
typedef int32_t  elf64_sword_t;
typedef uint64_t elf64_xword_t;
typedef int64_t elf64_sxword_t;

typedef struct _elf64_eident_ {
    union {
        uint8_t ei_mag[4]; // 7F 'E' 'L' 'F'
        uint32_t ei_mag_all;
    };
    uint8_t ei_class;
    uint8_t ei_data;
    uint8_t ei_version;
    uint8_t ei_osabi;
    uint8_t ei_abiversion;
    uint8_t ei_pad;
    uint8_t _reserved[6];
} elf64_eident_t;

typedef struct _elf64_header {
    elf64_eident_t  e_ident;
    elf64_half_t    e_type;
    elf64_half_t    e_machine;
    elf64_word_t    e_version;
    elf64_addr_t    e_entry;
    elf64_off_t     e_phoff;
    elf64_off_t     e_shoff;
    elf64_word_t    e_flags;
    elf64_half_t    e_ehsize;
    elf64_half_t    e_phentsize;
    elf64_half_t    e_phnum;
    elf64_half_t    e_shentsize;
    elf64_half_t    e_shnum;
    elf64_half_t    e_shstrndx;
} elf64_header_t;

typedef struct _elf64_section_header {
    elf64_word_t    sh_name;
    elf64_word_t    sh_type;
    elf64_xword_t   sh_flags;
    elf64_addr_t    sh_addr;
    elf64_off_t     sh_offset;
    elf64_xword_t   sh_size;
    elf64_word_t    sh_link;
    elf64_word_t    sh_info;
    elf64_xword_t   sh_addralign;
    elf64_xword_t   sh_entsize;
} elf64_section_header_t;

typedef struct _elf64_program_header {
    elf64_word_t    p_type;
    elf64_word_t    p_flags;
    elf64_off_t     p_offset;
    elf64_addr_t    p_vaddr;
    elf64_addr_t    p_paddr;
    elf64_xword_t   p_filesz;
    elf64_xword_t   p_memsz;
    elf64_xword_t   p_align;
} elf64_program_header_t;

/*
typedef struct _elf64_sym_entry {
    elf64_word_t    st_name;
    uint8_t         st_info;
    uint8_t         st_other;
    elf64_half_t    st_shndx;
    elf64_addr_t    st_value;
    elf64_xword_t   st_size;
} elf64_sym_entry_t;
*/

#endif
