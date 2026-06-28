//
// Created by denis on 6/21/2026.
//

#include "mouse.h"

#include "../include/stdint.h"
#include "../include/util.h"
#include "../interrupts/idt.h"
#include "../display/display.h"
#include "../memory/malloc.h"
#include "../stdlib/serial.h"

void init_mouse() {
    uint8_t status;

    // --- STEP 1: Enable the Mouse Auxiliary Device Port ---
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xA8); // Enable Auxiliary (Mouse) Port

    // --- STEP 2: Enable IRQ12 in the Controller Config Byte ---
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0x20); // Command 0x20: Read Controller Command Byte

    while ((inPortB(0x64) & 0x01) == 0) {}
    status = inPortB(0x60); // Read the current configuration byte

    status |= 0x02;  // Set Bit 1: Enable IRQ 12 for mouse
    status &= ~0x20; // Clear Bit 5: Disable Mouse Clock Disable (meaning enable clock)

    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0x60); // Command 0x60: Write Controller Command Byte

    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, status); // Send updated status byte

    // --- STEP 3: Tell the mouse itself to reset and start streaming data ---
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xD4);
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, 0xF6); // Set defaults
    while ((inPortB(0x64) & 0x01) == 0) {}
    char ack1 = inPortB(0x60);

    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xD4);
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, 0xF4); // Enable data reporting
    while ((inPortB(0x64) & 0x01) == 0) {}
    char ack2 = inPortB(0x60);

    serial_printf("Installed mouse at irq 12\r\n");
    irq_install_handler(12, &mouseHandler);
}

uint8_t mouse_packet[3];
uint8_t mouse_packet_size = 0;

uint16_t __mx, __my = 0;

void* listeners[64];
uint8_t listener_count = 0;

uint8_t mouse_add_listener(void* listener) {
    if (listener_count >= 64) {
        serial_printf("Too many listeners\r\n");
        return 0;
    }
    listeners[listener_count] = listener;

    serial_printf("Added listener %x. Now got: %x\r\n", listener, listener_count+1);

    return listener_count++;
}

void mouse_remove_listener(uint8_t listener) {
    listeners[listener] = 0;
    for (uint8_t i = listener; i < listener_count; i++) {
        listeners[i] = listeners[i + 1];
    }
    listener_count--;
}

void mouse_notify_listeners(uint8_t flags) {
    for (uint8_t i = 0; i < listener_count; i++) {
        ((void (*)(uint8_t flags))listeners[i])(flags);
    }
}

void mouseHandler(struct InterruptRegisters *r) {

    mouse_packet[mouse_packet_size++] = inPortB(0x60);

    if (mouse_packet_size < 3) return;

    int8_t x = (int8_t)mouse_packet[1];
    int8_t y = (int8_t)mouse_packet[2];

    __mx += x;
    __my += -y;

    if (__mx < 0) __mx = 0;
    if (__my < 0) __my = 0;
    if (__mx >= SCREEN_WIDTH) __mx = SCREEN_WIDTH-1;
    if (__my >= SCREEN_HEIGHT) __my = SCREEN_HEIGHT-1;

    mouse_notify_listeners(mouse_packet[0]&0b111);

    mouse_packet_size = 0;
}