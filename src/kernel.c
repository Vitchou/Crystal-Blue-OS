#include "io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
unsigned char current_color = 0x1F; 

int cursor_x = 0;
int cursor_y = 0;
char shell_buffer[256];
int shell_pos = 0;

// Sécurité
int logged_in = 0;
char* secret_password = "admin";

// Timer
unsigned long timer_ticks = 0;

extern void idt_init();
void kputc(char c);
void kprint(char* str);
void kclear();

// --- Utilitaires ---

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

// --- NOUVEAU : Gestion du Timer ---

void timer_init(int hz) {
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void draw_timer() {
    char* vga = (char*)0xB8000;
    unsigned long seconds = timer_ticks / 100;
    
    // Affichage en haut à droite (position 70)
    int pos = (0 * VGA_WIDTH + 65) * 2;
    char time_str[10];
    
    vga[pos] = 'U'; vga[pos+2] = 'p'; vga[pos+4] = ':'; vga[pos+6] = ' ';
    
    // Conversion simple nombre -> char (pour les secondes)
    int s = seconds % 60;
    int m = (seconds / 60) % 60;
    
    vga[pos+8] = (m / 10) + '0';
    vga[pos+10] = (m % 10) + '0';
    vga[pos+12] = ':';
    vga[pos+14] = (s / 10) + '0';
    vga[pos+16] = (s % 10) + '0';
}

void timer_handler() {
    timer_ticks++;
    if (timer_ticks % 100 == 0) {
        draw_timer();
    }
    outb(0x20, 0x20); // EOI
}

// --- Fonctions classiques ---

void print_time() {
    unsigned char sec = read_cmos(0x00);
    unsigned char min = read_cmos(0x02);
    unsigned char hour_bcd = read_cmos(0x04);
    int hour = ((hour_bcd >> 4) & 0x0F) * 10 + (hour_bcd & 0x0F);
    hour = (hour + 1) % 24;
    kprint("Heure actuelle : ");
    kputc((hour / 10) + '0'); kputc((hour % 10) + '0'); kputc(':');
    kputc(((min >> 4) & 0x0F) + '0'); kputc((min & 0x0F) + '0');
}

void execute_command() {
    shell_buffer[shell_pos] = '\0';
    kprint("\n");
    if (strcmp(shell_buffer, "help") == 0) {
        kprint("Commandes: help, cls, time, whoami, cpu, logout, reboot, halt");
    } else if (strcmp(shell_buffer, "cls") == 0) {
        kclear();
    } else if (strcmp(shell_buffer, "time") == 0) {
        print_time();
    } else if (strcmp(shell_buffer, "logout") == 0) {
        logged_in = 0; kclear(); kprint("Session terminee.\nPassword: ");
        shell_pos = 0; return;
    } else if (strcmp(shell_buffer, "reboot") == 0) {
        outb(0x64, 0xFE);
    } else if (strcmp(shell_buffer, "halt") == 0) {
        asm volatile("hlt");
    } else if (shell_pos > 0) {
        kprint("Inconnu.");
    }
    kprint("\n> ");
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
                    kprint("Crystal Blue OS v0.6.0\nConnecte. Tapez 'help'.\n> ");
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
    timer_init(100); // 100 Hz
    
    kprint("Crystal Blue OS v0.6.0\n");
    kprint("----------------------\n");
    kprint("Password: ");
    
    while(1) {
        asm volatile("hlt");
    }
}