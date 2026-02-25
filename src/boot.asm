[BITS 16]
[ORG 0x7C00]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Charger le Kernel
    mov ah, 0x02
    mov al, 30
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x00
    mov bx, 0x1000
    int 0x13

    ; Passage en Mode Protégé
    lgdt [gdtr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

gdt_start:
    dd 0x0, 0x0
    dw 0xFFFF, 0x0
    db 0x0, 0x9A, 0xCF, 0x0
    dw 0xFFFF, 0x0
    db 0x0, 0x92, 0xCF, 0x0
gdt_end:
gdtr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp 0x1000    ; On saute directement au Kernel

times 510-($-$$) db 0
dw 0xAA55