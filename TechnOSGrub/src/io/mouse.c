//
// Created by denis on 6/21/2026.
//

#include "mouse.h"

#include "../include/stdint.h"
#include "../include/util.h"
#include "../interrupts/idt.h"
#include "../display/display.h"
#include "../memory/malloc.h"

void init_mouse() {
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xD4);

    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, 0xF6);

    while ((inPortB(0x64) & 0x01) == 0) {}
    char ack1 = inPortB(0x60);

    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xD4);

    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, 0xF4);

    while ((inPortB(0x64) & 0x01) == 0) {}
    char ack2 = inPortB(0x60);

    irq_install_handler(12, &mouseHandler);
}

uint8_t mouse_packet[3];
uint8_t mouse_packet_size = 0;

uint16_t mx, my = 0;

void* listeners[64];
uint8_t listener_count = 0;

uint8_t mouse_add_listener(void* listener) {
    listeners[listener_count] = listener;
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

void mouseHandler() {
    mouse_packet[mouse_packet_size++] = inPortB(0x60);

    if (mouse_packet_size < 3) return;

    int8_t x = (int8_t)mouse_packet[1];
    int8_t y = (int8_t)mouse_packet[2];

    mx += x;
    my += y;

    mouse_notify_listeners(mouse_packet[0]&0b111);

    if (mx < 0) mx = 0;
    if (my < 0) my = 0;
    if (mx > SCREEN_WIDTH) mx = SCREEN_WIDTH;
    if (my > SCREEN_HEIGHT) my = SCREEN_HEIGHT;

    mouse_packet_size = 0;
}