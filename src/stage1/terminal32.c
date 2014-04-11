#include "terminal32.h"
#include "util32.h"
#include "kernel32.h"

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

#define KTERM_BUFFER_SIZE (20480)
uint16_t kterm_buffer[KTERM_BUFFER_SIZE];
uint32_t kterm_buffer_pos;
uint32_t kterm_buffer_flushed_pos;
uint32_t kterm_buffer_linecount;
uint16_t kterm_color;

void kterm_clear() {
    volatile uint16_t *out = VGA_OUT_PTR;
    for (uint32_t i=0; i<VGA_SIZE; i++) {
        *out++ = BLANK_CHAR;
    }
}

void kterm_flush() {
    /*
    uint32_t poscopy = kterm_buffer_pos;
    uint32_t nn = 79;
    while (poscopy) {
        kterm_header[nn] = 48 + poscopy % 10;
        poscopy /= 10;
        nn--;
    }
    */
    volatile uint16_t *out = VGA_OUT_PTR;
    //write header line
    for (uint32_t i=0; i<VGA_WIDTH; i++)
        *out++ = make_vga_entry(kterm_header[i], kterm_color);
    
    uint32_t row = 0;
    uint32_t col = 0;
    
    if (kterm_buffer_flushed_pos > kterm_buffer_pos)
        kernel32_hang();
    
    for (uint32_t i=kterm_buffer_flushed_pos; i<kterm_buffer_pos; i++) {
        if ((kterm_buffer[i] & 0xFF) == '\n') {
            for (; col < VGA_WIDTH; col++)
                out[col+row*VGA_WIDTH] = BLANK_CHAR;
            row++;
            col = 0;
            if (row == VGA_HEIGHT-2) {
                uint32_t advance = 2;
                for (uint32_t ii=kterm_buffer_flushed_pos; ii<kterm_buffer_pos; ii++) {
                    if (kterm_buffer[i] == '\n') {
                        advance--;
                        kterm_buffer_flushed_pos = ii+1;
                        if (advance == 0)
                            break;
                    }
                }
                //kernel32_hang();
            }
            
        } else {
            #if KTERM_WRAP
                out[col+row*VGA_WIDTH] = kterm_buffer[i];
                col++;
                if (col == VGA_WIDTH)
                    col = 0;
            #else
                if (col < VGA_WIDTH) {
                    out[col+row*VGA_WIDTH] = kterm_buffer[i];
                    col++;
                }
            #endif
        }
    }
    for (; col < VGA_WIDTH; col++)
        out[col+row*VGA_WIDTH] = BLANK_CHAR;
}

/*
 *  public
 */

void kterm_initialize() {
    //char h[80] = "[ ] SNRK OS...cpu=..............................................................";
    char h[80] = "[ ] SNRK OS.....................................................................";
    for (uint32_t i=0; i<80; i++)
        kterm_header[i] = h[i];
    kterm_buffer_pos = 0;
    kterm_buffer_flushed_pos = 0;
    kterm_buffer_linecount = 0;
    kterm_color = make_vga_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    kterm_clear();
    kterm_flush();
}

void kterm_set_color(enum kterm_vga_color fg_color, enum kterm_vga_color bg_color) {
    kterm_color = make_vga_color(fg_color, bg_color);
}


void kterm_write_char(char c) {
    kterm_buffer[kterm_buffer_pos++] = make_vga_entry(c, kterm_color);
    
    if (kterm_buffer_pos == KTERM_BUFFER_SIZE) {
        kernel32_error_code = 0x442;
        kernel32_hang();
    }
    
    if (c == '\n') {
        kterm_buffer_linecount++;
        if (kterm_buffer_linecount >= VGA_HEIGHT_M1) {
            while ((kterm_buffer[++kterm_buffer_flushed_pos] & 0xFF) != '\n');
            kterm_buffer_flushed_pos++;
        }
        kterm_flush();
    }
}

void kterm_write(const char *str) {
    uint32_t len = strlen(str), flush = 0;
    if (kterm_buffer_pos + len >= KTERM_BUFFER_SIZE) {
        kernel32_error_code = 0x442;
        kernel32_hang();
    }
    while (--len) {
        char c = *str++;
        if (c == '\n') {
            kterm_buffer_linecount++;
            if (kterm_buffer_linecount >= VGA_HEIGHT_M1) {
                while ((kterm_buffer[++kterm_buffer_flushed_pos] & 0xFF) != '\n');
                kterm_buffer_flushed_pos++;
            }
            flush = 1;
        }
        kterm_buffer[kterm_buffer_pos++] = make_vga_entry(c, kterm_color);
    }
    if (flush)
        kterm_flush();
}

uint32_t kterm_write_if(uint32_t n, enum kterm_write_mode mode, uint8_t sz, uint8_t base, uint8_t len) {
    return kterm_write_ifp((void*)(&n), mode, sz, base, len);
}

uint32_t kterm_write_ifp(void* n, enum kterm_write_mode mode, uint8_t sz, uint8_t base, uint8_t len) {
    
    if (base < 2 || base > 36) {
        return 1;
    }
    
    if (len == 0)
        return 1;
    
    uint32_t i = 0, j = 0;
    
    char abc[66];
    char dig[66];
    
    uint32_t err = 0;
    uint32_t x;
    switch (sz) {
        case 1: x = *(uint8_t*)n; break;
        case 2: x = *(uint16_t*)n; break;
        case 4: x = *(uint32_t*)n; break;
        default: err = 1; break;
    }
    
    if (err) {
        return 1;
    }
    
    uint32_t neg = 0;
    
    if (mode & BIT_2) {
        switch (sz) {
            case 1:
                if (x & BIT_7) {
                    neg = 1;
                    x ^= BIT_7;
                }
            break;
            case 2:
                if (x & BIT_15) {
                    neg = 1;
                    x ^= BIT_15;
                }
            break;
            case 4:
                if (x & BIT_31) {
                    neg = 1;
                    x ^= BIT_31;
                }
            break;
        }
    }
    
    if (neg)
        abc[j++] = '-';
    
    /*
    if (((mode & 3) != 3) && (mode & BIT_3)) {
        switch (base) {
            case 2:
                abc[j++] = '0';
                abc[j++] = 'b';
            break;
            case 10:
                abc[j++] = '0';
                abc[j++] = 'd';
            break;
            case 8:
                abc[j++] = '0';
                abc[j++] = 'o';
            break;
            case 16:
                abc[j++] = '0';
                abc[j++] = 'x';
            break;
            default:
                abc[j++] = '0';
                abc[j++] = '[';
                abc[j++] = asciify(base / 10);
                abc[j++] = asciify(base % 10);
                abc[j++] = ']';
            break;
        }
    }
    */
    
    do {
        dig[i++] = x % base;
        x /= base;
    } while (x || (i < len));
    
    switch (mode & 3) {
        case 0:
            if (mode & BIT_3) {
                switch (base) {
                    case 2:
                        abc[j++] = '0';
                        abc[j++] = 'b';
                    break;
                    case 10:
                        abc[j++] = '0';
                        abc[j++] = 'd';
                    break;
                    case 8:
                        abc[j++] = '0';
                        abc[j++] = 'o';
                    break;
                    case 16:
                        abc[j++] = '0';
                        abc[j++] = 'x';
                    break;
                    default:
                        abc[j++] = '0';
                        abc[j++] = '[';
                        abc[j++] = asciify(base / 10);
                        abc[j++] = asciify(base % 10);
                        abc[j++] = ']';
                    break;
                }
            }
            for (i--; i > 0; i--) {
                if (dig[i])
                    break;
            }
        break;
        case 1:
            if (mode & BIT_3) {
                switch (base) {
                    case 2:
                        abc[j++] = '0';
                        abc[j++] = 'b';
                    break;
                    case 10:
                        abc[j++] = '0';
                        abc[j++] = 'd';
                    break;
                    case 8:
                        abc[j++] = '0';
                        abc[j++] = 'o';
                    break;
                    case 16:
                        abc[j++] = '0';
                        abc[j++] = 'x';
                    break;
                    default:
                        abc[j++] = '0';
                        abc[j++] = '[';
                        abc[j++] = asciify(base / 10);
                        abc[j++] = asciify(base % 10);
                        abc[j++] = ']';
                    break;
                }
            }
            for (i--; i > 0; i--) {
                if (dig[i])
                    break;
                abc[j++] = '0';
            }
        break;
        case 3:
            for (i--; i > 0; i--) {
                if (dig[i])
                    break;
                abc[j++] = ' ';
            }
            if (mode & BIT_3) {
                switch (base) {
                    case 2:
                        abc[j++] = '0';
                        abc[j++] = 'b';
                    break;
                    case 10:
                        abc[j++] = '0';
                        abc[j++] = 'd';
                    break;
                    case 8:
                        abc[j++] = '0';
                        abc[j++] = 'o';
                    break;
                    case 16:
                        abc[j++] = '0';
                        abc[j++] = 'x';
                    break;
                    default:
                        abc[j++] = '0';
                        abc[j++] = '[';
                        abc[j++] = asciify(base / 10);
                        abc[j++] = asciify(base % 10);
                        abc[j++] = ']';
                    break;
                }
            }
        break;
    }
    
    do {
        abc[j++] = asciify(dig[i]);
    } while (i--);
    
    abc[j] = '\0';
    
    kterm_write(abc);
    
    return 0;
}

uint32_t kterm_row_size() {
    return VGA_WIDTH;
}
