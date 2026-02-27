#include "include/io.h"
#include "include/common.h"
#include "include/vga.h"
#include "include/nano.h"

void update_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;

    outb(0x3D4, 0x0F); outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E); outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}


void kscroll() {
    char* vga = (char*)0xB8000;

    int start_y = (nano_active) ? 1 : 0;
    int end_y = (nano_active) ? VGA_HEIGHT - 1 : VGA_HEIGHT;

    for (int y = start_y; y < end_y - 1; y++) {
        for(int x=0; x < VGA_WIDTH * 2; x++) {
            vga[y * VGA_WIDTH * 2 + x] = vga[(y + 1) * VGA_WIDTH * 2 + x];
        }
    }
    

    int last_line = (end_y - 1) * VGA_WIDTH * 2;
    for (int i = 0; i < VGA_WIDTH * 2; i += 2) {
        vga[last_line + i] = ' ';
        vga[last_line + i + 1] = (nano_active) ? 0x07 : current_color;
    }
    cursor_y = end_y - 1;
}


void kputc(char c) {
    char* vga = (char*)0xB8000;
    
    if (c == '\n') {
        cursor_x = 0; 
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) { 
            cursor_x--; 
            vga[(cursor_y * VGA_WIDTH + cursor_x) * 2] = ' '; 
        }
    } else {
        int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        vga[pos] = c;
        vga[pos+1] = (nano_active) ? 0x07 : current_color;
        cursor_x++;
    }


    if (cursor_x >= VGA_WIDTH) { 
        cursor_x = 0; 
        cursor_y++; 
    }
    

    int max_y = (nano_active) ? VGA_HEIGHT - 1 : VGA_HEIGHT;
    if (cursor_y >= max_y) kscroll();
    
    update_cursor();
}


void kprint(char* str) {
    for (int i = 0; str[i] != 0; i++) kputc(str[i]);
}


void kclear() {
    char* vga = (char*)0xB8000;
    unsigned char color = (nano_active) ? 0x07 : current_color;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga[i] = ' '; 
        vga[i+1] = color;
    }
    cursor_x = 0; 
    cursor_y = 0; 
    update_cursor();
}

void kprint_int(int n) {
    char buf[12]; 
    int i = 10; 
    buf[11] = '\0';
    
    if (n == 0) { kputc('0'); return; }
    if (n < 0) { kputc('-'); n = -n; }
    
    while (n > 0 && i >= 0) { 
        buf[i--] = (n % 10) + '0'; 
        n /= 10; 
    }
    kprint(&buf[i + 1]);
}