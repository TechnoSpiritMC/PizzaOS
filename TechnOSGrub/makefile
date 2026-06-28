SRC = src
BUILD = build

# Compilers
gcc = /usr/opt/cross/bin/i686-elf-gcc
gxx = /usr/opt/cross/bin/i686-elf-g++

# Compiler Flags
CFLAGS = -ffreestanding -Wall -Wextra -g -O2
CXXFLAGS = -ffreestanding -Wall -Wextra -g -O2 -fno-exceptions -fno-rtti

all: gxx gcc asm ld iso

# 1. Added build/oop.o to the objects list
ld:
	ld -m elf_i386 -T linker.ld -o $(BUILD)/kernel \
	$(BUILD)/boot.o $(BUILD)/kernel.o $(BUILD)/vga.o \
	$(BUILD)/gdts.o $(BUILD)/gdt.o $(BUILD)/util.o \
	$(BUILD)/idt.o $(BUILD)/idts.o $(BUILD)/timer.o \
	$(BUILD)/stdio.o $(BUILD)/keyboard.o $(BUILD)/console.o \
	$(BUILD)/memory.o $(BUILD)/kmalloc.o $(BUILD)/serial.o \
	$(BUILD)/display.o $(BUILD)/disk.o $(BUILD)/malloc.o \
	$(BUILD)/oop.o $(BUILD)/mouse.o $(BUILD)/IApplication.o \
	$(BUILD)/screen.o $(BUILD)/list.o $(BUILD)/SampleApp.o

asm: $(SRC)/asm/boot.asm $(SRC)/asm/gdt.asm
	nasm -f elf32 $(SRC)/asm/boot.asm       -o $(BUILD)/boot.o
	nasm -f elf32 $(SRC)/asm/gdt.asm        -o $(BUILD)/gdts.o
	nasm -f elf32 $(SRC)/interrupts/idt.asm -o $(BUILD)/idts.o

# 2. Added a target for compiling your C++ files
gxx: $(SRC)/memory/oop.cpp
	$(gxx) $(CXXFLAGS) -c $(SRC)/memory/oop.cpp -o $(BUILD)/oop.o
	$(gxx) $(CXXFLAGS) -c $(SRC)/display/screen.cpp -o $(BUILD)/screen.o
	$(gxx) $(CXXFLAGS) -c $(SRC)/apps/IApplication.cpp -o $(BUILD)/IApplication.o
	$(gxx) $(CXXFLAGS) -c $(SRC)/apps/SampleApp.cpp -o $(BUILD)/SampleApp.o
	$(gxx) $(CXXFLAGS) -c $(SRC)/stdlib/list.cpp -o $(BUILD)/list.o

gcc: $(SRC)/kernel.c $(SRC)/vga.c $(SRC)/gdt.c $(SRC)/util.c $(SRC)/interrupts/idt.c $(SRC)/timer/timer.c $(SRC)/stdlib/stdio.c $(SRC)/stdlib/serial.c $(SRC)/console.c $(SRC)/memory/memory.c $(SRC)/memory/kmalloc.c $(SRC)/memory/malloc.c $(SRC)/display/display.c $(SRC)/io/disk/disk.c
	$(gcc) $(CFLAGS) -c $(SRC)/kernel.c           -o $(BUILD)/kernel.o
	$(gcc) $(CFLAGS) -c $(SRC)/vga.c              -o $(BUILD)/vga.o
	$(gcc) $(CFLAGS) -c $(SRC)/gdt.c              -o $(BUILD)/gdt.o
	$(gcc) $(CFLAGS) -c $(SRC)/util.c             -o $(BUILD)/util.o
	$(gcc) $(CFLAGS) -c $(SRC)/interrupts/idt.c   -o $(BUILD)/idt.o
	$(gcc) $(CFLAGS) -c $(SRC)/timer/timer.c      -o $(BUILD)/timer.o
	$(gcc) $(CFLAGS) -c $(SRC)/stdlib/stdio.c     -o $(BUILD)/stdio.o
	$(gcc) $(CFLAGS) -c $(SRC)/io/keyboard.c      -o $(BUILD)/keyboard.o
	$(gcc) $(CFLAGS) -c $(SRC)/stdlib/serial.c    -o $(BUILD)/serial.o
	$(gcc) $(CFLAGS) -c $(SRC)/console.c          -o $(BUILD)/console.o
	$(gcc) $(CFLAGS) -c $(SRC)/memory/memory.c    -o $(BUILD)/memory.o
	$(gcc) $(CFLAGS) -c $(SRC)/memory/kmalloc.c   -o $(BUILD)/kmalloc.o
	$(gcc) $(CFLAGS) -c $(SRC)/memory/malloc.c    -o $(BUILD)/malloc.o
	$(gcc) $(CFLAGS) -c $(SRC)/display/display.c  -o $(BUILD)/display.o
	$(gcc) $(CFLAGS) -c $(SRC)/io/disk/disk.c     -o $(BUILD)/disk.o
	$(gcc) $(CFLAGS) -c $(SRC)/io/mouse.c         -o $(BUILD)/mouse.o

iso: $(BUILD)/kernel
	cp $(BUILD)/kernel Pizza/boot/kernel
	grub-mkrescue -o Pizza.iso Pizza

clean:
	clear
	rm -rf $(BUILD)/*

run:
	make
	qemu-system-i386 -cdrom Pizza.iso

disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=64

clean-disk:
	rm -f disk.img

rundrive: disk.img
	make
	qemu-system-i386 -cdrom Pizza.iso -drive file=disk.img,format=raw,if=ide