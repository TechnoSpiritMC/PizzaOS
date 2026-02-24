#include "include/vga.h"
#include "include/gdt.h"
#include "interrupts/idt.h"
#include "stdlib/keyboard.h"
#include "timer/timer.h"
#include "include/console.h"

void kmain(void);

void kmain(void) {
    Reset();

    print("Initializing processes..\r\n");
    initGdt();
    initIdt();
    initTimer();
    initKeyboard();
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
