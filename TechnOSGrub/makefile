# =========================
# Directories
# =========================
SRC := src
BUILD := build

# =========================
# Toolchain
# =========================
gcc := /usr/opt/cross/bin/i686-elf-gcc
gxx := /usr/opt/cross/bin/i686-elf-g++
ld  := ld

NASM := nasm

# =========================
# Flags
# =========================
CFLAGS := -ffreestanding -Wall -Wextra -g -O2
CXXFLAGS := -ffreestanding -Wall -Wextra -g -O2 -fno-exceptions -fno-rtti

# =========================
# Auto parallel build
# =========================
MAKEFLAGS += -j$(shell nproc)

# =========================
# Source files (EASY TO EDIT)
# Add new files HERE ONLY
# =========================

C_SOURCES := \
kernel.c \
vga.c \
gdt.c \
util.c \
interrupts/idt.c \
timer/timer.c \
stdlib/stdio.c \
stdlib/serial.c \
console.c \
memory/memory.c \
memory/kmalloc.c \
memory/malloc.c \
display/display.c \
io/disk/disk.c \
io/keyboard.c \
io/mouse.c

CPP_SOURCES := \
memory/oop.cpp \
display/screen.cpp \
apps/IApplication.cpp \
apps/SampleApp.cpp \
stdlib/list.cpp

ASM_SOURCES := \
asm/boot.asm \
asm/gdt.asm \
interrupts/idts.asm

# =========================
# Object files (auto generated)
# =========================
C_OBJS    := $(C_SOURCES:%.c=$(BUILD)/%.o)
CPP_OBJS  := $(CPP_SOURCES:%.cpp=$(BUILD)/%.o)
ASM_OBJS  := $(ASM_SOURCES:%.asm=$(BUILD)/%.o)

OBJS := $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)

# =========================
# Default target
# =========================
all: iso

# =========================
# Build kernel
# =========================
$(BUILD)/kernel: $(OBJS)
	$(ld) -m elf_i386 -T linker.ld -o $@ $^

# =========================
# C compilation rule
# =========================
$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(gcc) $(CFLAGS) -c $< -o $@

# =========================
# C++ compilation rule
# =========================
$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(gxx) $(CXXFLAGS) -c $< -o $@

# =========================
# ASM compilation rule
# =========================
$(BUILD)/%.o: $(SRC)/%.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

# =========================
# ISO generation
# =========================
iso: $(BUILD)/kernel
	cp $(BUILD)/kernel Pizza/boot/kernel
	grub-mkrescue -o Pizza.iso Pizza

# =========================
# Cleanup
# =========================
clean:
	clear
	rm -rf $(BUILD)/*

# =========================
# Run targets
# =========================
runnodrive: iso
	qemu-system-i386 -cdrom Pizza.iso

disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=64

clean-disk:
	rm -f disk.img

rundrive: disk.img iso
	qemu-system-i386 -cdrom Pizza.iso -drive file=disk.img,format=raw,if=ide -display gtk -serial vc