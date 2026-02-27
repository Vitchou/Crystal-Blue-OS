#include "include/memory.h"
#include "include/vga.h"
#include "include/string.h"

extern unsigned int _end;

static block_header_t* head = 0;

static unsigned int heap_start = 0;

static unsigned int align4(unsigned int n) {
    return (n + 3) & ~3;
}

void memory_init() {
    heap_start = align4((unsigned int)&_end);
    
    head = (block_header_t*)heap_start;
    head->size = HEAP_END - heap_start - sizeof(block_header_t);
    head->is_free = 1;
    head->next = 0;
    head->magic = BLOCK_MAGIC;
}

void* kmalloc(unsigned int size) {

    if (size == 0) return 0;
    
    size = align4(size);  
    
    block_header_t* current = head;
    
    while (current) {
        if (current->magic != BLOCK_MAGIC) {
            kprint("PANIC: Corruption memoire detectee dans kmalloc!\n");
            return 0;
        }
        
        if (current->is_free && current->size >= size) {
            unsigned int min_split = sizeof(block_header_t) + 4;
            
            if (current->size >= size + min_split) {

                block_header_t* new_block = (block_header_t*)((unsigned char*)current + sizeof(block_header_t) + size);
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->magic = BLOCK_MAGIC;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            
            return (void*)((unsigned char*)current + sizeof(block_header_t));
        }
        
        current = current->next;
    }
    
    kprint("ERREUR: kmalloc - Plus de memoire!\n");
    return 0;
}

void kfree(void* ptr) {

    if (!ptr) return;
    
    block_header_t* block = (block_header_t*)((unsigned char*)ptr - sizeof(block_header_t));
    
    if (block->magic != BLOCK_MAGIC) {
        kprint("PANIC: kfree sur un pointeur invalide ou corrompu!\n");
        return;
    }
    
    if (block->is_free) {
        kprint("ATTENTION: Double free detecte!\n");
        return;
    }
    
    block->is_free = 1;
    
    block_header_t* current = head;
    while (current) {
        if (current->magic != BLOCK_MAGIC) break;
        
        if (current->is_free && current->next && current->next->is_free) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
            continue;
        }
        current = current->next;
    }
}

void memory_stats() {
    // Affiche le nombre de blocs, la mémoire utilisée et libre.
    // Utile pour la commande "ram" du shell.
    block_header_t* current = head;
    unsigned int total_free = 0;
    unsigned int total_used = 0;
    int block_count = 0;
    int free_blocks = 0;
    int used_blocks = 0;
    
    while (current) {
        if (current->magic != BLOCK_MAGIC) {
            kprint("ATTENTION: Bloc corrompu detecte dans les stats!\n");
            break;
        }
        
        block_count++;
        if (current->is_free) {
            total_free += current->size;
            free_blocks++;
        } else {
            total_used += current->size;
            used_blocks++;
        }
        current = current->next;
    }
    
    kprint("=== Memoire Crystal Blue OS ===\n");
    
    kprint("Heap debut : 0x");
    unsigned int addr = heap_start;
    char hex[9];
    for (int i = 7; i >= 0; i--) {
        int nibble = addr & 0xF;
        hex[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        addr >>= 4;
    }
    hex[8] = '\0';
    kprint(hex);
    kprint("\n");
    
    kprint("Heap fin   : 0x400000\n");
    
    kprint("Blocs totaux : "); kprint_int(block_count);
    kprint(" ("); kprint_int(used_blocks); kprint(" occupes, ");
    kprint_int(free_blocks); kprint(" libres)\n");
    
    kprint("Memoire utilisee : "); kprint_int(total_used / 1024); kprint(" KB (");
    kprint_int(total_used); kprint(" octets)\n");
    
    kprint("Memoire libre    : "); kprint_int(total_free / 1024); kprint(" KB (");
    kprint_int(total_free); kprint(" octets)\n");
    
    unsigned int total = total_free + total_used;
    kprint("Heap total       : "); kprint_int(total / 1024); kprint(" KB\n");
}