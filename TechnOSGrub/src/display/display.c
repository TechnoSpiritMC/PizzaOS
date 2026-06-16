//
// Created by denis on 6/16/2026.
//

#include "display.h"
#include "../include/vga.h"
#include "../memory/memory.h"
#include "../stdlib/serial.h"

uint32_t* framebuffer;
int fb_pitch;

void draw_string(int px, int py, const char* s, uint32_t fg, uint32_t bg, font_id_t font) {
    int cursor_x = px;
    while (*s) {
        if (*s == '\n' || *s == '\r') {
            py += font_height(font);
            cursor_x = px;
        } else {
            draw_char(cursor_x, py, *s, fg, bg, font);
            cursor_x += font_width(font);
        }
        s++;
    }
}

void initDisplay(struct multiboot_info* bootInfo) {
    bootInfo = (struct multiboot_info*)((uint32_t)bootInfo + 0xC0000000);

    struct vbe_mode_info_structure* vbe = (struct vbe_mode_info_structure*)(bootInfo->vbe_mode_info + 0xC0000000);
    serial_printf("Loaded vbe.\r\n");
    uint32_t physAddr = vbe->framebuffer;
    serial_printf("Loaded physAdress: %x\r\n", physAddr);
    uint32_t virtAddr = 0xE0000000;
    serial_printf("Loaded virtual adress: %x\r\n", virtAddr);

    serial_printf("Mapping screen at 0xE0000000 (physical BIOS address %x)...\r\n", physAddr);

    for (int i = 0; i < 469; i++) {
        memMapPage(virtAddr, physAddr, PAGE_FLAG_WRITE);
        physAddr += 0x1000;
        virtAddr += 0x1000;
    }

    memChangePageDir(memGetCurrentPageDir());

    print("Mapped screen!\r\n");
    serial_printf("Mapped screen!\r\n");

    uint32_t* pixelDisplay = (uint32_t*)0xE0000000;

    for (int i = 0; i < 800 * 600; i++) {
        pixelDisplay[i] = 0x00000022;
    }

    framebuffer = pixelDisplay;
    fb_pitch = vbe->pitch / (vbe->bpp / 8); // Check if this is working?
    // fb_pitch = 800; // Check if this is working?
}