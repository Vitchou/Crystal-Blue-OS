#include "io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
unsigned char current_color = 0x1F; 

int cursor_x = 0;
int cursor_y = 0;
char shell_buffer[256];
int shell_pos = 0;

int logged_in = 0;
char* secret_password = "admin";

unsigned long timer_ticks = 0;

extern void idt_init();
void kputc(char c);
void kprint(char* str);
void kclear();

int strcmp(char* s1, char* s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 0;
        i++;
    }
    return s1[i] - s2[i];
}

int strncmp(char* s1, char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

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
    if (timer_ticks % 10 == 0) {
        draw_clock();
    }
    outb(0x20, 0x20); 
}

void print_time() {
    unsigned char day = read_cmos(0x07);
    unsigned char month = read_cmos(0x08);
    unsigned char year = read_cmos(0x09);
    kprint("Date du systeme : ");
    kputc(((day >> 4) & 0x0F) + '0'); kputc((day & 0x0F) + '0'); kputc('/');
    kputc(((month >> 4) & 0x0F) + '0'); kputc((month & 0x0F) + '0'); kprint("/20");
    kputc(((year >> 4) & 0x0F) + '0'); kputc((year & 0x0F) + '0');
}

void execute_command() {
    shell_buffer[shell_pos] = '\0';
    kprint("\n");

    if (strcmp(shell_buffer, "help") == 0) {
        kprint("Commandes: help, cls, time, whoami, cpu, echo [txt], logout, reboot, halt");
    } 
    else if (strcmp(shell_buffer, "cls") == 0 || strcmp(shell_buffer, "clear") == 0) {
        kclear();
    } 
    else if (strcmp(shell_buffer, "time") == 0) {
        print_time();
    } 
    else if (strcmp(shell_buffer, "whoami") == 0) {
        kprint("Utilisateur: Root\nMachine: Crystal-T14-Virtual");
    } 
    else if (strcmp(shell_buffer, "cpu") == 0) {
        kprint("Processeur: x86 compatible (Mode Protege + IDT + PIT)");
    } 
    else if (strncmp(shell_buffer, "echo ", 5) == 0) {
        kprint(shell_buffer + 5);
    } 
    else if (strcmp(shell_buffer, "logout") == 0) {
        logged_in = 0; 
        kclear(); 
        kprint("Session terminee.\nPassword: ");
        shell_pos = 0; 
        return;
    } 
    else if (strcmp(shell_buffer, "reboot") == 0) {
        outb(0x64, 0xFE);
    } 
    else if (strcmp(shell_buffer, "halt") == 0) {
        kprint("Systeme stoppe.");
        asm volatile("hlt");
    } 
    else if (shell_pos > 0) {
        kprint("Erreur: '"); kprint(shell_buffer); kprint("' inconnu.");
    }

    if (strcmp(shell_buffer, "cls") != 0 && strcmp(shell_buffer, "logout") != 0) {
        kprint("\n> ");
    } else if (strcmp(shell_buffer, "cls") == 0) {
        kprint("> ");
    }

    shell_pos = 0;
}   

void update_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F); outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E); outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void kputc(char c) {
    char* vga = (char*)0xB8000;
    if (c == '\n') {
        cursor_x = 0; cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) { cursor_x--; vga[(cursor_y * VGA_WIDTH + cursor_x) * 2] = ' '; }
    } else {
        int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        vga[pos] = c; vga[pos+1] = current_color;
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
    update_cursor();
}

void kprint(char* str) {
    for (int i = 0; str[i] != 0; i++) kputc(str[i]);
}

void kclear() {
    char* vga = (char*)0xB8000;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga[i] = ' '; vga[i+1] = current_color;
    }
    cursor_x = 0; cursor_y = 0; update_cursor();
}

unsigned char get_ascii(unsigned char scancode) {
    static unsigned char azerty_map[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ')', '=', '\b',
        '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',
        0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', '%', '*', 0,
        '*', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, '*', 0, ' '
    };
    return (scancode < 128) ? azerty_map[scancode] : 0;
}

void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    char c = get_ascii(scancode);
    if (c > 0) {
        if (c == '\n') {
            if (!logged_in) {
                shell_buffer[shell_pos] = '\0';
                if (strcmp(shell_buffer, secret_password) == 0) {
                    logged_in = 1; kclear();
                    kprint("Crystal Blue OS v0.6.1\nConnecte. Tapez 'help'.\n> ");
                } else { kprint("\nErreur.\nPassword: "); }
                shell_pos = 0;
            } else { execute_command(); }
        } else if (c == '\b') {
            if (shell_pos > 0) { shell_pos--; kputc('\b'); }
        } else if (shell_pos < 255) {
            shell_buffer[shell_pos++] = c;
            if (!logged_in) kputc('*'); else kputc(c);
        }
    }
    outb(0x20, 0x20); 
}

void __attribute__((section(".text.kernel_main"))) kernel_main() {
    kclear();
    idt_init();
    timer_init(100); 
    
    kprint("Crystal Blue OS v0.6.1\n");
    kprint("----------------------\n");
    kprint("Password: ");
    
    while(1) {
        asm volatile("hlt");
    }
}