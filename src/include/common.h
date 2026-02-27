#ifndef COMMON_H
#define COMMON_H

extern unsigned char current_color;
extern int cursor_x;
extern int cursor_y;
extern unsigned long timer_ticks;
extern int logged_in;
extern char secret_password[32];
extern char admin_username[32];
extern int shift_pressed;

extern int login_step;
extern char username_buffer[32];
extern int username_pos;

extern char shell_buffer[256];
extern int shell_pos;

void draw_login_screen();
void login_show_error(char* msg);
void show_login_password_prompt();

#endif