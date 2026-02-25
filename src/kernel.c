#include "io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
unsigned char current_color = 0x1F; 

int cursor_x = 0;
int cursor_y = 0;
char shell_buffer[256];
int shell_pos = 0;

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

void print_time() {
    unsigned char sec = read_cmos(0x00);
    unsigned char min = read_cmos(0x02);
    unsigned char hour_bcd = read_cmos(0x04);
    unsigned char day = read_cmos(0x07);
    unsigned char month = read_cmos(0x08);
    unsigned char year = read_cmos(0x09);
    int hour = ((hour_bcd >> 4) & 0x0F) * 10 + (hour_bcd & 0x0F);
    hour = hour + 1; 
    if (hour >= 24) hour = 0;
    kprint("Date : ");
    kputc(((day >> 4) & 0x0F) + '0'); kputc((day & 0x0F) + '0'); kputc('/');
    kputc(((month >> 4) & 0x0F) + '0'); kputc((month & 0x0F) + '0'); kprint("/20");
    kputc(((year >> 4) & 0x0F) + '0'); kputc((year & 0x0F) + '0');
    kprint(" - Heure : ");
    kputc((hour / 10) + '0'); kputc((hour % 10) + '0'); kputc(':');
    kputc(((min >> 4) & 0x0F) + '0'); kputc((min & 0x0F) + '0'); kputc(':');
    kputc(((sec >> 4) & 0x0F) + '0'); kputc((sec & 0x0F) + '0');
}

// --- Shell ---

void execute_command() {
    shell_buffer[shell_pos] = '\0';
    kprint("\n");
    if (strcmp(shell_buffer, "help") == 0) {
        kprint("Commandes: help, cls, time, version, whoami, cpu, echo [txt], color [1/2/4/e], reboot, halt");
    } else if (strcmp(shell_buffer, "cls") == 0 || strcmp(shell_buffer, "clear") == 0) {
        kclear();
    } else if (strcmp(shell_buffer, "time") == 0) {
        print_time();
    } else if (strcmp(shell_buffer, "version") == 0) {
        kprint("Crystal Blue OS v0.4.0 - Interrupt Mode");
    } else if (strcmp(shell_buffer, "whoami") == 0) {
        kprint("Utilisateur: Root\nMachine: Crystal-T14-Virtual");
    } else if (strcmp(shell_buffer, "cpu") == 0) {
        kprint("Processeur: x86 compatible (IDT Active)");
    } else if (strncmp(shell_buffer, "echo ", 5) == 0) {
        kprint(shell_buffer + 5);
    } else if (strncmp(shell_buffer, "color ", 6) == 0) {
        char c = shell_buffer[6];
        if (c == '1') current_color = 0x1F;
        else if (c == '2') current_color = 0x2F;
        else if (c == '4') current_color = 0x4F;
        else if (c == 'e') current_color = 0xEF;
        kprint("Couleur modifiee. Tapez 'cls' pour rafraichir.");
    } else if (strcmp(shell_buffer, "reboot") == 0) {
        outb(0x64, 0xFE);
    } else if (strcmp(shell_buffer, "halt") == 0) {
        kprint("Systeme stoppe.");
        asm volatile("hlt");
    } else if (shell_pos > 0) {
        kprint("Erreur: '"); kprint(shell_buffer); kprint("' inconnu.");
    }
    kprint("\n> ");
    shell_pos = 0;
}

// --- Affichage ---

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
        if (cursor_x > 2) {
            cursor_x--;
            vga[(cursor_y * VGA_WIDTH + cursor_x) * 2] = ' ';
        }
    } else {
        int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        vga[pos] = c;
        vga[pos+1] = current_color;
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= VGA_HEIGHT) { cursor_y = 0; kclear(); } 
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

// --- Gestionnaire d'Interruption ---

void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    char c = get_ascii(scancode);
    if (c > 0) {
        if (c == '\n') execute_command();
        else if (c == '\b') { if (shell_pos > 0) { shell_pos--; kputc('\b'); } }
        else if (shell_pos < 255) { shell_buffer[shell_pos++] = c; kputc(c); }
    }
    outb(0x20, 0x20); // Fin d'interruption (EOI)
}

// --- Point d'entrée ---

void __attribute__((section(".text.kernel_main"))) kernel_main() {
    kclear();
    kprint("Crystal Blue OS v0.4.0\n");
    
    kprint("Initialisation IDT... ");
    idt_init();
    kprint("OK!\n");
    
    kprint("Mode Interruptions actif.\n> ");
    
    while(1) {
        asm volatile("hlt");
    }
}