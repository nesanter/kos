#include "early_kterm.h"

//this is technically only valid on color monitors
#define VGA_OUT_PTR (uint16_t*)(0xB8000)

/*
 *  text mode constants
 */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_HEIGHT_M1 (VGA_HEIGHT-1)
#define VGA_SIZE (VGA_WIDTH*VGA_HEIGHT)

/*
 *  macros
 */

#define asciify(c) (c < 10 ? (c+48) : (c+55))
#define make_vga_color(fg, bg) (fg | (bg << 4))
#define make_vga_entry(c, color) ((uint16_t)c | ((uint16_t)color << 8))

#define BLANK_CHAR make_vga_entry(' ', make_vga_color(COLOR_LIGHT_GREY, COLOR_BLACK))

/* 
 *  private
 */

uint16_t *kterm_out = VGA_OUT_PTR;
uint32_t kterm_col;
uint32_t kterm_row;
uint16_t kterm_color;

/*
 *  public
 */

void ekterm_clear() {
    volatile uint16_t *out = VGA_OUT_PTR;
    for (uint32_t i=0; i<VGA_SIZE; i++) {
        *out++ = BLANK_CHAR;
    }
    kterm_col = 0;
    kterm_row = 0;
}

void ekterm_initialize() {
    kterm_color = make_vga_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    ekterm_clear();
}

void ekterm_set_color(enum kterm_vga_color fg_color, enum kterm_vga_color bg_color) {
    kterm_color = make_vga_color(fg_color, bg_color);
}

void ekterm_write_char(char c) {
    if (c == '\n') {
        kterm_row++;
        kterm_col = 0;
    } else {
        kterm_out[kterm_row * VGA_WIDTH + kterm_col] = make_vga_entry(c, kterm_color);
        kterm_col++;
        if (kterm_col == VGA_WIDTH) {
            kterm_row++;
            kterm_col = 0;
        }
    }
    if (kterm_row == VGA_HEIGHT) {
        ekterm_clear();
    }
}

void ekterm_write(const char *str) {
    while (1) {
        char c = *str++;
        if (c == '\0')
            break;
        ekterm_write_char(c);
    }
}

void ekterm_write_hex(uint64_t n, uint64_t dig) {
    char str[17];
    str[dig] = '\0';
    while (dig--) {
        str[dig] = asciify((n & 0xF));
        n >>= 4;
    }
    ekterm_write(str);
}

void ekterm_write_dec(uint64_t n) {
    char str[21];
    
    uint64_t dig = 1, tmp = n;
    while (tmp) {
        dig++;
        tmp /= 10;
    }
    
    str[dig] = '\0';
    while (dig--) {
        str[dig] = asciify((n % 10));
        n /= 10;
    }
    
    ekterm_write(str);
}

void ekterm_write_sdec(int64_t n) {
    char str[22];
    
    if (n < 0) {
        str[0] = '-';
        n *= -1;
    }
    
    uint64_t dig = 1, tmp = n;
    while (tmp) {
        dig++;
        tmp /= 10;
    }
    
    str[dig] = '\0';
    while (dig--) {
        str[dig] = asciify((n % 10));
        n /= 10;
    }
    
    ekterm_write(str);
}

void ekterm_write_bin(uint64_t n, uint64_t dig) {
    char str[65];
    str[dig] = '\0';
    while (dig--) {
        str[dig] = asciify((n & 0x1));
        n >>= 1;
    }
    ekterm_write(str);
}
