[BITS 32]
global idt_load
global keyboard_handler_asm
global timer_handler_asm
extern idtp
extern keyboard_handler
extern timer_handler

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

timer_handler_asm:
    pushad
    call timer_handler
    popad
    iretd

keyboard_handler_asm:
    pushad
    call keyboard_handler
    popad
    iretd