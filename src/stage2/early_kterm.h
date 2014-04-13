#ifndef _EKTERM_H
#define _EKTERM_H

#include <stddef.h>
#include <stdint.h>

enum kterm_vga_color
{
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
};

enum kterm_write_mode
{
    ALIGN_NONE      = 0b0000,
    ALIGN_ZERO      = 0b0001,
    ALIGN_BLANK     = 0b0011,
    S_ALIGN_NONE    = 0b0100,
    S_ALIGN_ZERO    = 0b0101,
    S_ALIGN_BLANK   = 0b0111,
    
    ALIGN_NONE_P    = 0b1000,
    ALIGN_ZERO_P    = 0b1001,
    ALIGN_BLANK_P   = 0b1011,
    S_ALIGN_NONE_P  = 0b1100,
    S_ALIGN_ZERO_P  = 0b1101,
    S_ALIGN_BLANK_P = 0b1111,
};

void ekterm_clear();
void ekterm_initialize();
void ekterm_set_color(enum kterm_vga_color fg_color, enum kterm_vga_color bg_color);
void ekterm_write_char(char c);
void ekterm_write(const char *str);
void ekterm_write_hex(uint64_t n, uint64_t dig);
void ekterm_write_dec(uint64_t n);
void ekterm_write_sdec(int64_t n);
void ekterm_write_bin(uint64_t n, uint64_t dig);

#endif
