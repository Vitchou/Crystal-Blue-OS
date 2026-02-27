#include "include/io.h"
#include "include/common.h"
#include "include/vga.h"
#include "include/timer.h"
#include "include/nano.h"

unsigned char read_cmos(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void timer_init(int hz) {
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void draw_clock() {
    if(nano_active) return; 
    
    char* vga = (char*)0xB8000;
    
    unsigned char sec_bcd = read_cmos(0x00);
    unsigned char min_bcd = read_cmos(0x02);
    unsigned char hour_bcd = read_cmos(0x04);
    
    int s = ((sec_bcd >> 4) & 0x0F) * 10 + (sec_bcd & 0x0F);
    int m = ((min_bcd >> 4) & 0x0F) * 10 + (min_bcd & 0x0F);
    int h = ((hour_bcd >> 4) & 0x0F) * 10 + (hour_bcd & 0x0F);
    

    h = (h + 1) % 24; 

    int pos = (0 * VGA_WIDTH + 71) * 2;
    unsigned char clock_color = 0x1E;

    vga[pos]   = (h / 10) + '0'; vga[pos+1] = clock_color;
    vga[pos+2] = (h % 10) + '0'; vga[pos+3] = clock_color;
    vga[pos+4] = ':';            vga[pos+5] = clock_color;
    vga[pos+6] = (m / 10) + '0'; vga[pos+7] = clock_color;
    vga[pos+8] = (m % 10) + '0'; vga[pos+9] = clock_color;
    vga[pos+10] = ':';           vga[pos+11] = clock_color;
    vga[pos+12] = (s / 10) + '0'; vga[pos+13] = clock_color;
    vga[pos+14] = (s % 10) + '0'; vga[pos+15] = clock_color;
}


void timer_handler() {
    timer_ticks++;
    

    if (timer_ticks % 100 == 0) {
        draw_clock();
    }
    

    outb(0x20, 0x20); 
}