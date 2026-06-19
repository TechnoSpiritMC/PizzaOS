#pragma once

#include "../include/stdint.h"
#include "../include/multiboot.h"
#include "fontDispatch.h"

extern uint32_t* framebuffer;
extern int fb_pitch;

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static inline uint32_t blend(uint32_t fg, uint32_t bg, uint8_t alpha /*0-255*/) {
    uint8_t fr = (fg >> 16) & 0xFF, fgg = (fg >> 8) & 0xFF, fb_ = fg & 0xFF;
    uint8_t br = (bg >> 16) & 0xFF, bgg = (bg >> 8) & 0xFF, bb_ = bg & 0xFF;
    uint8_t r = (fr * alpha + br * (255 - alpha)) / 255;
    uint8_t g = (fgg * alpha + bgg * (255 - alpha)) / 255;
    uint8_t b = (fb_ * alpha + bb_ * (255 - alpha)) / 255;
    return (r << 16) | (g << 8) | b;
}

static inline void draw_char(int px, int py, char c, uint32_t fg, uint32_t bg, font_id_t font) {
    int16_t idx = font_glyph_idx(font, c);
    if (idx < 0) return; /* unknown char, draw nothing or a placeholder box */

    for (int y = 0; y < font_height(font); y++) {
        for (int x = 0; x < font_width(font); x++) {
            uint8_t gray = font_pixel(font, idx, x, y);
            uint32_t color = blend(fg, bg, gray);
            // uint32_t color = (gray > 127) ? fg : bg; // linear b/w
            framebuffer[(py + y) * fb_pitch + (px + x)] = color;
        }
    }
}

static inline void draw_pixel(int px, int py, uint32_t color) {
    framebuffer[py * fb_pitch + px] = color;
}

void draw_string(int px, int py, const char* s, uint32_t fg, uint32_t bg, font_id_t font);
void initDisplay(struct multiboot_info* bootInfo);
void testDisplayAndFonts();