#include "include/io.h"
#include "include/common.h"
#include "include/vga.h"
#include "include/keyboard.h"
#include "include/nano.h"
#include "include/shell.h"
#include "include/string.h"

#define LOGIN_FIELD_MAX 15

unsigned char get_ascii(unsigned char scancode) {
    static unsigned char azerty_map[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ')', '=', '\b',
        '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',
        0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', '%', '*', 0,
        '*', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, '*', 0, ' '
    };
    return (scancode < 128) ? azerty_map[scancode] : 0;
}

static void handle_login_input(char c) {
    if (login_step == 0) {
        if (c == '\n') {
            username_buffer[username_pos] = '\0';
            login_step = 1;
            shell_pos = 0;
            show_login_password_prompt();
        } else if (c == '\b') {
            if (username_pos > 0) {
                username_pos--;
                kputc('\b');
            }
        } else if (c >= ' ' && c < 127 && username_pos < LOGIN_FIELD_MAX) {
            if (shift_pressed && c >= 'a' && c <= 'z') c -= 32;
            username_buffer[username_pos++] = c;
            kputc(c);
        }
    } else {
        if (c == '\n') {
            shell_buffer[shell_pos] = '\0';
            if (strcmp(username_buffer, admin_username) == 0 &&
                strcmp(shell_buffer, secret_password) == 0) {
                logged_in = 1;
                login_step = 0;
                current_color = 0x1F;
                kclear();
                kprint("Connecte. Tapez 'help'.\n> ");
            } else {
                login_show_error("Identifiants incorrects !");
                username_pos = 0;
                username_buffer[0] = '\0';
                shell_pos = 0;
                login_step = 0;
                kprint("\n\n    Utilisateur : ");
            }
        } else if (c == '\b') {
            if (shell_pos > 0) {
                shell_pos--;
                kputc('\b');
            }
        } else if (c >= ' ' && c < 127 && shell_pos < LOGIN_FIELD_MAX) {
            if (shift_pressed && c >= 'a' && c <= 'z') c -= 32;
            shell_buffer[shell_pos++] = c;
            kputc('*');
        }
    }
}

void keyboard_handler() {
    unsigned char scancode = inb(0x60);

    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; outb(0x20, 0x20); return; }
    if (scancode == 0xAA || scancode == 0xB6) { shift_pressed = 0; outb(0x20, 0x20); return; }
    if (scancode >= 0x80) { outb(0x20, 0x20); return; }

    if (nano_active && scancode == 0x01) {
        nano_exit_and_save();
        outb(0x20, 0x20);
        return;
    }

    char c = get_ascii(scancode);
    if (c == 0) { outb(0x20, 0x20); return; }

    if (!logged_in) {
        handle_login_input(c);
    } else if (nano_active) {
        if (c == '\b') {
            if (nano_pos > 0) { nano_pos--; kputc('\b'); }
        } else if (nano_pos < 511) {
            nano_buffer[nano_pos++] = c;
            kputc(c);
        }
    } else {
        if (c == '\n') {
            execute_command();
        } else if (c == '\b') {
            if (shell_pos > 0) { shell_pos--; kputc('\b'); }
        } else if (shell_pos < 255) {
            if (shift_pressed && c >= 'a' && c <= 'z') c -= 32;
            shell_buffer[shell_pos++] = c;
            kputc(c);
        }
    }

    outb(0x20, 0x20);
}