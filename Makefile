all: build/os.img

build/os.img: src/boot.asm src/kernel.c src/idt.c src/interrupt.asm
	mkdir -p build
	# Bootloader binaire
	nasm src/boot.asm -f bin -o build/boot.bin
	# Objets Kernel
	gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -c src/kernel.c -o build/kernel.o
	gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -c src/idt.c -o build/idt.o
	nasm src/interrupt.asm -f elf32 -o build/interrupt.o
	# LE LIEN CRITIQUE : on lie TOUT ensemble
	ld -m elf_i386 -T src/linker.ld -o build/kernel.bin build/kernel.o build/idt.o build/interrupt.o --oformat binary
	# Assemblage
	cat build/boot.bin build/kernel.bin > build/os.img
	truncate -s 1440k build/os.img

run: build/os.img
	qemu-system-i386 -fda build/os.img

clean:
	rm -rf build