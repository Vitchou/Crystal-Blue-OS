[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, msg1
    call print_string

    mov ah, 0x02
    mov al, 20
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x00
    mov bx, 0x1000
    int 0x13

    jc disk_error

    mov si, msg2
    call print_string

    jmp load_gdt

disk_error:
    mov si, msg_err
    call print_string
    hlt

print_string:
    lodsb
    cmp al, 0
    je done
    mov ah, 0x0E
    int 0x10
    jmp print_string
done: ret

msg1    db 'Loading kernel...', 0x0D, 0x0A, 0
msg2    db 'Kernel loaded! Switching to PM...', 0x0D, 0x0A, 0
msg_err db 'Disk error!', 0

gdt_start:
    dd 0x0
    dd 0x0

    dw 0xFFFF
    dw 0x0
    db 0x0
    db 0x9A
    db 0xCF
    db 0x0

    dw 0xFFFF
    dw 0x0
    db 0x0
    db 0x92
    db 0xCF
    db 0x0

gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

load_gdt:
    lgdt [gdtr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov eax, 0x1000
    call eax
    
    hlt

times 510-($-$$) db 0
dw 0xAA55