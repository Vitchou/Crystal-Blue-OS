[BITS 32]
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    push byte %1
    jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21
; (On peut s'arrêter à 21 ou faire les 31, les autres sont réservées)

; Interruptions matérielles (Timer et Clavier)
global timer_handler_asm
global keyboard_handler_asm
extern timer_handler
extern keyboard_handler

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

; Stub commun pour les exceptions
extern exception_handler
isr_common_stub:
    pushad
    call exception_handler
    popad
    add esp, 8
    iretd

global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret