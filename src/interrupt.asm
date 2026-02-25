[BITS 32]
global idt_load
global keyboard_handler_asm
extern idtp
extern keyboard_handler

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

keyboard_handler_asm:
    pushad
    call keyboard_handler
    popad
    iretd