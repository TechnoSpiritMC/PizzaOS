// fontdispatch.h
#pragma once
#include "../include/stdint.h"
#include "../stdlib/serial.h"
#include "fonts/monospace1.h"

#include "fonts/monospace2.h"

#ifndef DEFAULT_FONT_GLYPH
#define DEFAULT_FONT_GLYPH font_monospace1_glyph
#endif

#ifndef DEFAULT_FONT_WIDTH
#define DEFAULT_FONT_WIDTH FONT_MONOSPACE1_WIDTH
#endif

#ifndef DEFAULT_FONT_HEIGHT
#define DEFAULT_FONT_HEIGHT FONT_MONOSPACE1_HEIGHT
#endif

#ifndef DEFAULT_FONT
#define DEFAULT_FONT MONOSPACE1
#endif

typedef enum {
    FONT_DEFAULT = 0,
    MONOSPACE1,
    MONOSPACE2_BIGGER,
    // FONT_CUSTOM,
    FONT_COUNT
} font_id_t;

static inline const char* font_name(font_id_t font) {
    switch (font) {
        case MONOSPACE1: return "Monospace1";
        case MONOSPACE2_BIGGER: return "Monospace2";
        default: return "Default";
    }
}

static inline const uint16_t font_glyph_idx(font_id_t font, char c) {

    //serial_printf("Trying to get glyph idx for font with id %x (%s). Chose: ", font, font_name(font));

    switch (font) {
        case MONOSPACE1: {
            //serial_printf("Monospace1\r\n");
            return font_monospace1_glyph_idx(c);
        }
        case MONOSPACE2_BIGGER: {
            //serial_printf("Monospace2\r\n");
            return font_monospace2_glyph_idx(c);
        }
        default: {
            //serial_printf("Default\r\n");
            return font_glyph_idx(DEFAULT_FONT, c);
        }
    }
}

static inline const uint8_t* font_glyph(font_id_t font, char c) {
    switch (font) {
        case MONOSPACE1:
            return font_monospace1_glyph(c);
        case MONOSPACE2_BIGGER:
            return font_monospace2_glyph(c);
        case FONT_DEFAULT:
        default:
            return DEFAULT_FONT_GLYPH(c);
    }
}

static inline int font_width(font_id_t font) {
    switch (font) {
        case MONOSPACE1:        return FONT_MONOSPACE1_WIDTH;
        case MONOSPACE2_BIGGER: return FONT_MONOSPACE2_WIDTH;
        default:                return DEFAULT_FONT_WIDTH;
    }
}

static inline int font_height(font_id_t font) {
    switch (font) {
        case MONOSPACE1:        return FONT_MONOSPACE1_HEIGHT;
        case MONOSPACE2_BIGGER: return FONT_MONOSPACE2_HEIGHT;
        default:                return DEFAULT_FONT_HEIGHT;
    }
}

static inline uint8_t font_pixel(font_id_t font, int16_t idx, int x, int y) {
    switch (font) {
        case MONOSPACE1:
            return FONT_MONOSPACE1_DATA[
                ((size_t)idx * FONT_MONOSPACE1_HEIGHT + y)
                * FONT_MONOSPACE1_WIDTH + x];

        case MONOSPACE2_BIGGER:
            return FONT_MONOSPACE2_DATA[
                ((size_t)idx * FONT_MONOSPACE2_HEIGHT + y)
                * FONT_MONOSPACE2_WIDTH + x];
        default: return font_pixel(DEFAULT_FONT, idx, x, y);
    }
}