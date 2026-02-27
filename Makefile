# --- CONFIGURATION ---
CC = gcc
ASM = nasm
LD = ld

# Flags de compilation (32-bit, freestanding, pas de protection de pile)
CFLAGS = -m32 -ffreestanding -fno-stack-protector -fno-pic -Isrc -Isrc/include
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T src/linker.ld --oformat binary

# --- LISTE DES OBJETS ---
# C'est ici qu'on liste tout ce qu'on a découpé
OBJS = build/kernel.o \
       build/idt.o \
       build/interrupt_asm.o \
       build/memory.o \
       build/drivers/vga.o \
       build/drivers/keyboard.o \
       build/drivers/timer.o \
       build/lib/string.o \
       build/fs/ramfs.o \
       build/app/nano.o \
       build/app/shell.o

# --- CIBLES PRINCIPALES ---

all: build/os.img

# Création de l'image finale
build/os.img: build/boot.bin build/kernel.bin
	cat build/boot.bin build/kernel.bin > build/os.img
	truncate -s 1440k build/os.img

# Compilation du Bootloader
build/boot.bin: src/boot.asm
	mkdir -p build
	$(ASM) src/boot.asm -f bin -o build/boot.bin

# Lien de tous les objets pour créer le Kernel binaire
build/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o build/kernel.bin $(OBJS)

# --- RÈGLES DE COMPILATION GÉNÉRIQUES ---

# Compilation des fichiers C à la racine (kernel.c, idt.c, memory.c)
build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation des fichiers C dans les sous-dossiers (drivers, lib, fs, app)
build/drivers/%.o: src/drivers/%.c
	mkdir -p build/drivers
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/%.o: src/lib/%.c
	mkdir -p build/lib
	$(CC) $(CFLAGS) -c $< -o $@

build/fs/%.o: src/fs/%.c
	mkdir -p build/fs
	$(CC) $(CFLAGS) -c $< -o $@

build/app/%.o: src/app/%.c
	mkdir -p build/app
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation de l'assembleur d'interruptions
build/interrupt_asm.o: src/interrupt.asm
	$(ASM) $(ASFLAGS) src/interrupt.asm -o build/interrupt_asm.o

# --- UTILITAIRES ---

run: build/os.img
	qemu-system-i386 -fda build/os.img

clean:
	rm -rf build