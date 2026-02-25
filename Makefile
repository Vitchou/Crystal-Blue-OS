all: build/os.img

build/os.img: src/boot.asm src/kernel.c
	mkdir -p build
	nasm src/boot.asm -f bin -o build/boot.bin
	gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -c src/kernel.c -o build/kernel.o
	ld -m elf_i386 -T src/linker.ld -o build/kernel.bin build/kernel.o --oformat binary
	cat build/boot.bin build/kernel.bin > build/os.img
	# On s'assure que le fichier fait au moins 1.44Mo (taille disquette)
	truncate -s 1440k build/os.img

run: build/os.img
	qemu-system-i386 -fda build/os.img

clean:
	rm -rf build

.PHONY: all run clean
