#include "include/vga.h"
#include "include/gdt.h"
#include "interrupts/idt.h"
#include "stdlib/keyboard.h"
#include "timer/timer.h"
#include "include/console.h"
#include "include/multiboot.h"
#include "memory/kmalloc.h"
#include "memory/memory.h"

#include "stdlib/stdio.h"

void kmain(uint32_t magic, struct multiboot_info* bootInfo);

void kmain(uint32_t magic, struct multiboot_info* bootInfo) {
    Reset();

    print("Initializing processes..\r\n");
    initGdt();
    initIdt();
    initTimer();
    initKeyboard();

    print("Initializing memory..\r\n");

    uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xfff) & ~0xfff;
    printf("mod1: %x, pas: %x\n", mod1, physicalAllocStart);
    initMemory(bootInfo->mem_upper*1024, physicalAllocStart);

    kmallocInit(0x1000);


    print("All services initialized successfully!\r\n");

#if newConsole
    print("Starting console in 3 seconds...\r\n");
    sleep(3);
    initConsole();
#endif

    for (;;) {
        __asm__ volatile("hlt");
    }
}
