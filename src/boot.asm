[BITS 16]
[ORG 0x7C00]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    mov ah, 0x02
    mov bh, 0
    mov dx, 0x0000
    int 0x10
    
    mov ah, 0x09
    mov al, 'B'
    mov bh, 0
    mov bl, 0x0F
    mov cx, 1
    int 0x10
    
    mov si, boot_msg
    call print_string

    mov ah, 0x02
    mov al, 30
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x00
    mov bx, 0x1000
    int 0x13

    lgdt [gdtr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode
jmp 0x08:protected_mode

error_load:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

boot_msg: db " LOAD...", 0
load_ok: db " OK PM", 0
error_msg: db " ERROR", 0

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
    jmp 0x1000

times 510-($-$$) db 0
dw 0xAA55