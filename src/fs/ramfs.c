#include "include/vga.h"
#include "include/string.h"
#include "include/fs.h"

File filesystem[MAX_FILES];

void fs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        filesystem[i].used = 0;
        filesystem[i].size = 0;
        filesystem[i].name[0] = '\0';
        for(int j = 0; j < MAX_FILE_SIZE; j++) filesystem[i].content[j] = 0;
    }
}

int fs_create(char* name) {
    if (name[0] == '\0') return -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && strcmp(filesystem[i].name, name) == 0) {
            return i; 
        }
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (!filesystem[i].used) {
            strcpy(filesystem[i].name, name);
            filesystem[i].used = 1;
            filesystem[i].size = 0;
            filesystem[i].content[0] = '\0';
            return i;
        }
    }

    return -1;
}

int fs_write(char* name, char* text) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && strcmp(filesystem[i].name, name) == 0) {
            int len = strlen(text);
            if (len >= MAX_FILE_SIZE) len = MAX_FILE_SIZE - 1;
            
            strcpy(filesystem[i].content, text);
            filesystem[i].size = len;
            return 0;
        }
    }
    return -1;
}

void fs_ls() {
    kprint("Index   Nom             Taille\n");
    kprint("-----   ---             ------\n");
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used) {
            kprint_int(i); kprint("       ");
            kprint(filesystem[i].name);
            
            int nlen = strlen(filesystem[i].name);
            if (nlen < 8) kprint("                "); 
            else kprint("        ");
            
            kprint_int(filesystem[i].size); kprint(" octets\n");
            count++;
        }
    }
    if (count == 0) kprint("(Systeme de fichiers vide)\n");
}

void fs_cat(char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && strcmp(filesystem[i].name, name) == 0) {
            kprint(filesystem[i].content);
            kprint("\n");
            return;
        }
    }
    kprint("Erreur: Fichier introuvable.\n");
}

void fs_rm(char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && strcmp(filesystem[i].name, name) == 0) {
            filesystem[i].used = 0;
            filesystem[i].name[0] = '\0';
            filesystem[i].size = 0;
            kprint("Fichier supprime.\n");
            return;
        }
    }
    kprint("Erreur: Impossible de supprimer.\n");
}