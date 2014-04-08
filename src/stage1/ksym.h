#ifndef _KSYM_H
#define _KSYM_H

#define KSYM32_MAX_SYMBOLS 100
#define KSYM32_MAX_NAME_LENGTH 31

typedef struct _ksym_entry {
    char name[KSYM32_MAX_NAME_LENGTH];
    uint8_t attr;
    uint64_t ptr;
} ksym_entry_t;

typedef struct _ksym_table_32 {
    uint32_t bootstrap64_ptr;
    uint32_t num_symbols;
    ksym_entry_t symbols[KSYM32_MAX_SYMBOLS];
} ksym_table_32_t;

#endif
