#include "io.h"
#include "common.h"
#include "vga.h"
#include "fs.h"
#include "string.h"
#include "shell.h"
#include "nano.h"
#include "timer.h"
#include "keyboard.h"
#include "memory.h"

unsigned char current_color = 0x1F; 
int cursor_x = 0;
int cursor_y = 0;
unsigned long timer_ticks = 0;
int logged_in = 0;
char secret_password[32] = "admin";
char admin_username[32] = "admin";
int shift_pressed = 0;

int login_step = 0;
char username_buffer[32];
int username_pos = 0;

extern void idt_init();

void exception_handler() {
    current_color = 0x4F; 
    kclear();
    kprint("!!! EXCEPTION MATERIELLE !!!");
    while(1) { asm volatile("hlt"); }
}

void draw_login_screen() {
    kclear();
    current_color = 0x1F;

    kprint("\n\n\n");
    kprint("    ===================================\n");
    kprint("       C R Y S T A L   B L U E   O S\n");
    kprint("              v 1 . 1 . 0\n");
    kprint("    ===================================\n");
    kprint("\n\n");
    kprint("    Utilisateur : ");
}

void login_show_error(char* msg) {
    unsigned char old = current_color;
    current_color = 0x1C;
    kprint("\n    ");
    kprint(msg);
    current_color = old;
}

void show_login_password_prompt() {
    kprint("\n    Mot de passe: ");
}

void __attribute__((section(".text.kernel_main"))) kernel_main() {
    kclear();
    idt_init();      
    timer_init(100); 
    memory_init();
    fs_init();       
    
    asm volatile("sti"); 

    login_step = 0;
    username_pos = 0;
    username_buffer[0] = '\0';
    
    draw_login_screen();

    while(1) {
        asm volatile("hlt");
    }
}