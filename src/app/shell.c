#include "include/common.h"
#include "include/vga.h"
#include "include/fs.h"
#include "include/string.h"
#include "include/nano.h"
#include "include/timer.h"
#include "include/io.h"
#include "include/shell.h"
#include "include/memory.h"

char shell_buffer[256];
int shell_pos = 0;

void detect_ram() {
    memory_stats();
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

void neofetch() {
    unsigned char logo_color = 0x0B; 
    unsigned char info_color = 0x0F; 
    unsigned char old_color = current_color;

    kprint("\n");
    current_color = logo_color; kprint("      _________      "); current_color = info_color; kprint("Root@Crystal-T14\n");
    current_color = logo_color; kprint("     |         |     "); current_color = info_color; kprint("----------------\n");
    current_color = logo_color; kprint("   __|_________|__   "); current_color = info_color; kprint("OS: Crystal Blue OS v1.1.0\n");
    current_color = logo_color; kprint("  |  ___________  |  "); current_color = info_color; kprint("Kernel: x86 Bare Metal\n");
    current_color = logo_color; kprint("  | |           | |  "); current_color = info_color; kprint("Uptime: "); kprint_int(timer_ticks / 100); kprint(" seconds\n");
    current_color = logo_color; kprint("  | |   _   _   | |  "); current_color = info_color; kprint("Storage: RAM-FS Active\n");
    
    current_color = old_color;
}

void execute_command() {
    shell_buffer[shell_pos] = '\0';
    kprint("\n");

    char* cmd = shell_buffer;
    char* arg = "";
    for (int i = 0; shell_buffer[i] != '\0'; i++) {
        if (shell_buffer[i] == ' ') {
            shell_buffer[i] = '\0';
            arg = shell_buffer + i + 1;
            break;
        }
    }

    if (strcmp(cmd, "help") == 0) {
        kprint("Systeme: help, cls, ram, neofetch, time, whoami, cpu, color, pass, crash, logout, reboot, halt\n");
        kprint("Fichiers: ls, touch, nano, cat, rm, write");
    } 
    else if (strcmp(cmd, "cls") == 0 || strcmp(cmd, "clear") == 0) kclear();
    else if (strcmp(cmd, "ram") == 0) detect_ram();
    else if (strcmp(cmd, "neofetch") == 0) neofetch();
    else if (strcmp(cmd, "time") == 0) print_time();
    else if (strcmp(cmd, "whoami") == 0) kprint("Utilisateur: Root\nMachine: Crystal-T14-Virtual");
    else if (strcmp(cmd, "cpu") == 0) kprint("Processeur: x86 compatible");
    else if (strcmp(cmd, "echo") == 0) kprint(arg);
    else if (strcmp(cmd, "color") == 0) {
        if (arg[0] == '1') current_color = 0x1F;     
        else if (arg[0] == '2') current_color = 0x2F; 
        else if (arg[0] == '3') current_color = 0x3F; 
        else if (arg[0] == '4') current_color = 0x4F; 
        kprint("Couleur modifiee.");
    }
    else if (strcmp(cmd, "pass") == 0) {
        if (arg[0] != '\0') {
            strcpy(secret_password, arg);
            kprint("Mot de passe change.");
        }
    }
    else if (strcmp(cmd, "crash") == 0) {
        int a = 1, b = 0; int c = a / b; (void)c; 
    }
    else if (strcmp(cmd, "logout") == 0) {
        logged_in = 0;
        login_step = 0;
        username_pos = 0;
        username_buffer[0] = '\0';
        shell_pos = 0;
        draw_login_screen();
        return;
    } 
    else if (strcmp(cmd, "reboot") == 0) outb(0x64, 0xFE);
    else if (strcmp(cmd, "halt") == 0) asm volatile("hlt");
    else if (strcmp(cmd, "ls") == 0) fs_ls();
    else if (strcmp(cmd, "nano") == 0) open_nano(arg);
    else if (strcmp(cmd, "touch") == 0) {
        if (fs_create(arg) != -1) kprint("Fichier cree.");
    }
    else if (strcmp(cmd, "cat") == 0) fs_cat(arg);
    else if (strcmp(cmd, "rm") == 0) fs_rm(arg);
    else if (strcmp(cmd, "write") == 0) {
        char* filename = arg;
        char* content = "";
        for(int j=0; arg[j] != '\0'; j++) {
            if(arg[j] == ' ') {
                arg[j] = '\0';
                content = arg + j + 1;
                break;
            }
        }
        if (fs_write(filename, content) == 0) kprint("Ok.");
    }
    else if (shell_pos > 0) {
        kprint("Erreur: '"); kprint(cmd); kprint("' inconnu.");
    }

    if (!nano_active && strcmp(cmd, "cls") != 0 && strcmp(cmd, "logout") != 0) kprint("\n> ");
    else if (!nano_active && strcmp(cmd, "cls") == 0) kprint("> ");
    
    shell_pos = 0;
}