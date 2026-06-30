#include "io/disk/disk.h"
#include "include/vga.h"
#include "include/gdt.h"
#include "interrupts/idt.h"
#include "io/keyboard.h"
#include "timer/timer.h"
#include "include/console.h"
#include "include/multiboot.h"
#include "memory/kmalloc.h"
#include "memory/memory.h"
#include "stdlib/serial.h"
#include "display/display.h"
#include "io/mouse.h"
#include "memory/malloc.h"

#include "stdlib/stdio.h"

extern void start_sample_app();

static uint16_t p_mouse_x, p_mouse_y = 0;

void onMouseMoved(uint8_t flags) {
    serial_printf("Got mouse heartbeat. Mouse is at: (%x, %x)\r\n", __mx, __my);

    LOG_LINE();
    draw_pixel(CLAMP(__mx, 0, 319), CLAMP(__my, 0, 239), 0x00ffffff);

    LOG_LINE();
    if (p_mouse_x != __mx || p_mouse_y != __my) {
        LOG_LINE();
        draw_pixel(CLAMP(p_mouse_x, 0, 319), CLAMP(p_mouse_y, 0, 239), 0x00000022);
    }

    LOG_LINE();
    p_mouse_x = __mx;
    p_mouse_y =__my;
}

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
    init_mouse();
    mouse_add_listener(onMouseMoved);

    asm volatile("sti");

    print("Initializing memory..\r\n");
    serial_printf("Initializing memory!\r\n");

    uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xfff) & ~0xfff;

    uint32_t safePhysicalStart = 0x01000000; // 16 MB boundary

    printf("mod1: %x, pas: %x\n", mod1, physicalAllocStart);
    serial_printf("mod1: %x, pas: %x\r\n", mod1, physicalAllocStart);
    // initMemory(bootInfo->mem_upper*1024, physicalAllocStart);
    initMemory(bootInfo->mem_upper*1024, safePhysicalStart);

    kmallocInit(0x1000);

    // Debug before video init
    struct malloc_block_struct* test_start = (struct malloc_block_struct*)0xD0000000;
    serial_printf("BEFORE initDisplay - Size: %x, Next: %x\r\n", test_start->size, test_start->next);

    initDisplay(bootInfo);

    // Debug after video init
    serial_printf("AFTER initDisplay - Size: %x, Next: %x\r\n", test_start->size, test_start->next);

    bool ataInitialized = init_ata();
    serial_printf("Initialized ata? %x\r\n", ataInitialized);

    uint32_t my_addr = malloc(sizeof(int));
    int *my_ptr = (int*)my_addr;
    *my_ptr = 1234; // Stores 1234 at that address
    serial_printf("my_ptr: %x\r\n", my_ptr);
    serial_printf("my_ptr->value: %x\r\n", *my_ptr);

    // 2. Writing a string or buffer
    uint32_t buf_addr = malloc(64);
    char *str = (char*)buf_addr;
    strcpy(str, "Hello OS World!");
    serial_printf("str: %s\r\n", str);
    print(str);

    free(buf_addr);
    free(my_addr);
    
    testDisplayAndFonts();

    serial_printf("###################################\r\nSTARTING APPS:\r\n\n\n\n\n");
    start_sample_app();

    while (1) {}

#if newConsole
    print("Starting console in 3 seconds...\r\n");
    sleep(3);
    initConsole();
#endif

    for (;;) {
        __asm__ volatile("hlt");
    }
}
