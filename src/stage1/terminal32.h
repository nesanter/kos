#ifndef __MODE_KERNEL32
#error "THIS HEADER IS STAGE1 ONLY!"
#endif

#ifndef _TERMINAL32_H
#define _TERMINAL32_H

#define KTERM_WRAP 0

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

#define KTERM_WIDTH 80

void kterm_initialize();
void kterm_set_color(enum kterm_vga_color fg_color, enum kterm_vga_color bg_color);
void kterm_write_char(char c);
#define kterm_write_line() kterm_write_char('\n')
void kterm_write(const char *str);
void kterm_write_multiline(const char *str);
uint32_t kterm_write_if(uint32_t n, enum kterm_write_mode mode, uint8_t sz, uint8_t base, uint8_t len);
uint32_t kterm_write_ifp(void* n, enum kterm_write_mode mode, uint8_t sz, uint8_t base, uint8_t len);

uint32_t kterm_row_size();

char kterm_header[KTERM_WIDTH];

//kterm_write_if macros

#define kterm_write_ui8h(n) kterm_write_if((uint32_t)n, ALIGN_ZERO, 1, 16, 2)
#define kterm_write_si8h(n) kterm_write_if((uint32_t)n, S_ALIGN_ZERO, 1, 16, 2)
#define kterm_write_ui16h(n) kterm_write_if((uint32_t)n, ALIGN_ZERO, 2, 16, 4)
#define kterm_write_si16h(n) kterm_write_if((uint32_t)n, S_ALIGN_ZERO, 2, 16, 4)
#define kterm_write_ui32h(n) kterm_write_if((uint32_t)n, ALIGN_ZERO, 4, 16, 8)
#define kterm_write_si32h(n) kterm_write_if((uint32_t)n, S_ALIGN_ZERO, 4, 16, 8)

#define kterm_write_ui8hx(n) kterm_write_if((uint32_t)n, ALIGN_ZERO_P, 1, 16, 2)
#define kterm_write_si8hx(n) kterm_write_if((uint32_t)n, S_ALIGN_ZERO_P, 1, 16, 2)
#define kterm_write_ui16hx(n) kterm_write_if((uint32_t)n, ALIGN_ZERO_P, 2, 16, 4)
#define kterm_write_si16hx(n) kterm_write_if((uint32_t)n, S_ALIGN_ZERO_P, 2, 16, 4)
#define kterm_write_ui32hx(n) kterm_write_if((uint32_t)n, ALIGN_ZERO_P, 4, 16, 8)
#define kterm_write_si32hx(n) kterm_write_if((uint32_t)n, S_ALIGN_ZERO_P, 4, 16, 8)

#define kterm_write_ui8b(n) kterm_write_if((uint32_t)n, ALIGN_ZERO, 1, 2, 8)
#define kterm_write_ui16b(n) kterm_write_if((uint32_t)n, ALIGN_ZERO, 2, 2, 16)
#define kterm_write_ui32b(n) kterm_write_if((uint32_t)n, ALIGN_ZERO, 4, 2, 32)

#define kterm_write_ui8bx(n) kterm_write_if((uint32_t)n, ALIGN_ZERO_P, 1, 2, 8)
#define kterm_write_ui16bx(n) kterm_write_if((uint32_t)n, ALIGN_ZERO_P, 2, 2, 16)
#define kterm_write_ui32bx(n) kterm_write_if((uint32_t)n, ALIGN_ZERO_P, 4, 2, 32)

#define kterm_write_ui8da(n) kterm_write_if((uint32_t)n, ALIGN_BLANK, 1, 10, 3)
#define kterm_write_si8da(n) kterm_write_if((uint32_t)n, S_ALIGN_BLANK, 1, 10, 3)
#define kterm_write_ui16da(n) kterm_write_if((uint32_t)n, ALIGN_BLANK, 2, 10, 5)
#define kterm_write_si16da(n) kterm_write_if((uint32_t)n, S_ALIGN_BLANK, 2, 10, 5)
#define kterm_write_ui32da(n) kterm_write_if((uint32_t)n, ALIGN_BLANK, 4, 10, 10)
#define kterm_write_si32da(n) kterm_write_if((uint32_t)n, S_ALIGN_BLANK, 4, 10, 10)

#define kterm_write_ui8d(n) kterm_write_if((uint32_t)n, ALIGN_NONE, 1, 10, 3)
#define kterm_write_si8d(n) kterm_write_if((uint32_t)n, S_ALIGN_NONE, 1, 10, 3)
#define kterm_write_ui16d(n) kterm_write_if((uint32_t)n, ALIGN_NONE, 2, 10, 5)
#define kterm_write_si16d(n) kterm_write_if((uint32_t)n, S_ALIGN_NONE, 2, 10, 5)
#define kterm_write_ui32d(n) kterm_write_if((uint32_t)n, ALIGN_NONE, 4, 10, 10)
#define kterm_write_si32d(n) kterm_write_if((uint32_t)n, S_ALIGN_NONE, 4, 10, 10)

//kterm_write_ifp macros

#define kterm_write_uip8h(n) kterm_write_ifp((void*)n, ALIGN_ZERO, 1, 16, 2)
#define kterm_write_sip8h(n) kterm_write_ifp((void*)n, S_ALIGN_ZERO, 1, 16, 2)
#define kterm_write_uip16h(n) kterm_write_ifp((void*)n, ALIGN_ZERO, 2, 16, 4)
#define kterm_write_sip16h(n) kterm_write_ifp((void*)n, S_ALIGN_ZERO, 2, 16, 4)
#define kterm_write_uip32h(n) kterm_write_ifp((void*)n, ALIGN_ZERO, 4, 16, 8)
#define kterm_write_sip32h(n) kterm_write_ifp((void*)n, S_ALIGN_ZERO, 4, 16, 8)

#define kterm_write_uip8hx(n) kterm_write_ifp((void*)n, ALIGN_ZERO_P, 1, 16, 2)
#define kterm_write_sip8hx(n) kterm_write_ifp((void*)n, S_ALIGN_ZERO_P, 1, 16, 2)
#define kterm_write_uip16hx(n) kterm_write_ifp((void*)n, ALIGN_ZERO_P, 2, 16, 4)
#define kterm_write_sip16hx(n) kterm_write_ifp((void*)n, S_ALIGN_ZERO_P, 2, 16, 4)
#define kterm_write_uip32hx(n) kterm_write_ifp((void*)n, ALIGN_ZERO_P, 4, 16, 8)
#define kterm_write_sip32hx(n) kterm_write_ifp((void*)n, S_ALIGN_ZERO_P, 4, 16, 8)

#define kterm_write_uip8b(n) kterm_write_ifp((void*)n, ALIGN_ZERO, 1, 2, 8)
#define kterm_write_uip16b(n) kterm_write_ifp((void*)n, ALIGN_ZERO, 2, 2, 16)
#define kterm_write_uip32b(n) kterm_write_ifp((void*)n, ALIGN_ZERO, 4, 2, 32)

#define kterm_write_uip8bx(n) kterm_write_ifp((void*)n, ALIGN_ZERO_P, 1, 2, 8)
#define kterm_write_uip16bx(n) kterm_write_ifp((void*)n, ALIGN_ZERO_P, 2, 2, 16)
#define kterm_write_uip32bx(n) kterm_write_ifp((void*)n, ALIGN_ZERO_P, 4, 2, 32)

#define kterm_write_uip8da(n) kterm_write_ifp((void*)n, ALIGN_BLANK, 1, 10, 3)
#define kterm_write_sip8da(n) kterm_write_ifp((void*)n, S_ALIGN_BLANK, 1, 10, 3)
#define kterm_write_uip16da(n) kterm_write_ifp((void*)n, ALIGN_BLANK, 2, 10, 5)
#define kterm_write_sip16da(n) kterm_write_ifp((void*)n, S_ALIGN_BLANK, 2, 10, 5)
#define kterm_write_uip32da(n) kterm_write_ifp((void*)n, ALIGN_BLANK, 4, 10, 10)
#define kterm_write_sip32da(n) kterm_write_ifp((void*)n, S_ALIGN_BLANK, 4, 10, 10)

#define kterm_write_uip8d(n) kterm_write_ifp((void*)n, ALIGN_NONE, 1, 10, 3)
#define kterm_write_sip8d(n) kterm_write_ifp((void*)n, S_ALIGN_NONE, 1, 10, 3)
#define kterm_write_uip16d(n) kterm_write_ifp((void*)n, ALIGN_NONE, 2, 10, 5)
#define kterm_write_sip16d(n) kterm_write_ifp((void*)n, S_ALIGN_NONE, 2, 10, 5)
#define kterm_write_uip32d(n) kterm_write_ifp((void*)n, ALIGN_NONE, 4, 10, 10)
#define kterm_write_sip32d(n) kterm_write_ifp((void*)n, S_ALIGN_NONE, 4, 10, 10)

#endif
