#ifndef SHELL_H
#define SHELL_H

void detect_ram();
void print_time();
void neofetch();
void execute_command();

extern char shell_buffer[256];
extern int shell_pos;

#endif