#ifndef FS_H
#define FS_H

#define MAX_FILES 16
#define MAX_FILENAME 16
#define MAX_FILE_SIZE 512

typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_FILE_SIZE];
    int size;
    int used;
} File;

extern File filesystem[MAX_FILES];

void fs_init();
int fs_create(char* name);
int fs_write(char* name, char* text);
void fs_ls();
void fs_cat(char* name);
void fs_rm(char* name);

#endif