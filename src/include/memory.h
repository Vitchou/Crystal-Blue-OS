#ifndef MEMORY_H
#define MEMORY_H

#define HEAP_END 0x400000

typedef struct block_header {
    unsigned int size;
    unsigned int is_free;
    struct block_header* next;
    unsigned int magic;
} block_header_t;

#define BLOCK_MAGIC 0xCB05A110

void  memory_init();
void* kmalloc(unsigned int size);
void  kfree(void* ptr);
void  memory_stats();

#endif