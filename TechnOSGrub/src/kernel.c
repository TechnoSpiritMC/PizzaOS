#include "disk/disk.h"
#include "include/vga.h"
#include "include/gdt.h"
#include "interrupts/idt.h"
#include "stdlib/keyboard.h"
#include "timer/timer.h"
#include "include/console.h"
#include "include/multiboot.h"
#include "memory/kmalloc.h"
#include "memory/memory.h"
#include "stdlib/serial.h"
#include "display/display.h"

#include "stdlib/stdio.h"

void kmain(uint32_t magic, struct multiboot_info* bootInfo);

void kmain(uint32_t magic, struct multiboot_info* bootInfo) {
    Reset();

    serial_init();

    if (bootInfo->flags & 1 << 11) {
        print("Bit 11 is set!");
        serial_printf("Bit 11 is set!\r\n");
    }

    print("Initializing processes..\r\n");
    serial_printf("Initializing processes!\r\n");
    initGdt();
    initIdt();
    initTimer();
    initKeyboard();
    init_ata();

    print("Initializing memory..\r\n");
    serial_printf("Initializing memory!\r\n");

    uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xfff) & ~0xfff;
    printf("mod1: %x, pas: %x\n", mod1, physicalAllocStart);
    serial_printf("mod1: %x, pas: %x\r\n", mod1, physicalAllocStart);
    initMemory(bootInfo->mem_upper*1024, physicalAllocStart);

    kmallocInit(0x1000);


    print("All services initialized successfully!\r\n");
    serial_printf("All services initialized successfully!\r\n");

#if newConsole
    print("Starting console in 3 seconds...\r\n");
    sleep(3);
    initConsole();
#endif

    print("Started mapping screen...\r\n");
    serial_printf("Starting to map screen...\r\n");

    initDisplay(bootInfo);

    draw_string(100, 50, "HELLO WORLD!, 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,;:!?./\\^$%&~\"#{}()|_^[]*-+<>@", 0x00FFFFFF, 0x00000022, MONOSPACE1);
    draw_string(100, 100, " !\"#$%&'()*+,-./0123456789:;<=>?\n@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 0x00FFFFFF, 0x00000022, MONOSPACE2_BIGGER);
    for (int i = 0; i < 10*FONT_MONOSPACE2_HEIGHT; i+=FONT_MONOSPACE2_HEIGHT) {
        draw_char(100+i, 150+i, CEIL_DIV(i, FONT_MONOSPACE2_HEIGHT)+0x20, 0x00ffffFF, 0x00000022, MONOSPACE2_BIGGER);
    }

    for (;;) {
        __asm__ volatile("hlt");
    }
}
