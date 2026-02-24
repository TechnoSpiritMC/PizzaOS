#include "timer.h"
#include "../include/util.h"
#include "../include/vga.h"
#include "../include/stdint.h"
#include "../interrupts/idt.h"

#define freq 100

uint32_t ticks = 0;

void initTimer() {
    ticks = 0;
    irq_install_handler(0, &onIrq0);
    print("Bound irq0 (Timer interrupt) to (void*)onIrq0.\r\n");

    uint32_t divisor = 1193180 / freq;

    outPortB(0x43, 0x36);
    outPortB(0x40, (uint8_t)divisor & 0xFF);
    outPortB(0x40, (uint8_t)(divisor >> 8) & 0xFF);
}

uint32_t getTicks() {
    return ticks;
}

uint32_t getFreq() {
    return freq;
}

void onIrq0(struct InterruptRegisters* regs) {
    ticks++;
    #if debug
    print("Timer ticked!\r\n");
    #endif
}

void sleep(uint64_t ms) {
    uint64_t start = ticks;
    while (ticks - start < ms * freq) { // Do not mind the error; ticks updates in the background because of a bios interrupt.
        __asm__ volatile("hlt");
    }
}