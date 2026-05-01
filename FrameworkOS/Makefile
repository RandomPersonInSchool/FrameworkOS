CC = gcc
AS = as
LD = ld

CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

all: run

boot.o: src/boot.s
	$(AS) --32 src/boot.s -o boot.o

kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o kernel.o

terminal.o: src/terminal.c
	$(CC) $(CFLAGS) -c src/terminal.c -o terminal.o

kernel.bin: boot.o kernel.o terminal.o
	$(LD) $(LDFLAGS) boot.o kernel.o terminal.o -o kernel.bin

iso: kernel.bin
	mkdir -p iso/boot
	cp kernel.bin iso/boot/kernel.bin
	grub-mkrescue -o FrameworkOS.iso iso

run: iso
	qemu-system-i386 -cdrom FrameworkOS.iso

clean:
	rm -f *.o *.bin *.iso