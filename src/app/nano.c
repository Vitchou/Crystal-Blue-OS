#include "include/common.h"
#include "include/vga.h"
#include "include/fs.h"
#include "include/string.h"
#include "include/nano.h"

int nano_active = 0;
char nano_buffer[512]; 
int nano_pos = 0;
char nano_filename[16];
int nano_file_idx = -1;

void draw_nano_ui() {
    char* vga = (char*)0xB8000;
    for(int x=0; x<VGA_WIDTH; x++) { 
        vga[x*2] = ' '; 
        vga[x*2+1] = 0x70;
    }
    int offset = (VGA_WIDTH/2) - 10;
    char* title = " CRYSTAL NANO v1.0 ";
    for(int i=0; title[i] != '\0'; i++) vga[(offset+i)*2] = title[i];

    int start_last_line = (VGA_HEIGHT-1)*VGA_WIDTH*2;
    for(int x=0; x<VGA_WIDTH; x++) { 
        vga[start_last_line + x*2] = ' '; 
        vga[start_last_line + x*2+1] = 0x70; 
    }
    char* help = " ESC: Sauver & Quitter | Mode Edition Direct ";
    for(int i=0; help[i] != '\0'; i++) vga[start_last_line + i*2] = help[i];
}

void nano_refresh_text() {
    cursor_x = 0; 
    cursor_y = 1;
    update_cursor();
    for(int i=0; i < nano_pos; i++) {
        kputc(nano_buffer[i]);
    }
}

void open_nano(char* filename) {
    if (filename[0] == '\0') {
        kprint("Usage: nano [nom_fichier]\n");
        return;
    }


    nano_file_idx = fs_create(filename);

    if(nano_file_idx == -1) {
        kprint("Erreur: FS plein.\n");
        return;
    }

    nano_active = 1;
    strcpy(nano_filename, filename);
    
    for(int i=0; i<512; i++) nano_buffer[i] = 0;
    strcpy(nano_buffer, filesystem[nano_file_idx].content);
    nano_pos = strlen(nano_buffer);

    kclear();
    draw_nano_ui();
    nano_refresh_text();
}

void nano_exit_and_save() {
    nano_buffer[nano_pos] = '\0';
    fs_write(nano_filename, nano_buffer);
    
    nano_active = 0;
    kclear();
    current_color = 0x1F; 
    kprint("Fichier '"); kprint(nano_filename); kprint("' enregistre.\n> ");
}