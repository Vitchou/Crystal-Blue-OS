#ifndef NANO_H
#define NANO_H

extern int nano_active;
extern char nano_buffer[512];
extern int nano_pos;
extern char nano_filename[16];
extern int nano_file_idx;

void open_nano(char* filename);
void draw_nano_ui();
void nano_refresh_text();
void nano_exit_and_save();

#endif