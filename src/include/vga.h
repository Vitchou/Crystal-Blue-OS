#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void kputc(char c);
void kprint(char* str);
void kprint_int(int n);
void kclear();
void update_cursor();
void kscroll();

#endif  