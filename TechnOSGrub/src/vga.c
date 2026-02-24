#include "include/vga.h"
#include "include/util.h"

#ifdef __CLION_IDE__
#define far
#endif

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const) 0xB8000;
const uint16_t defaultColor = (color8_Bright_White << 8) | (color8_Black << 12);
uint16_t currentColor = defaultColor;

enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI
};

int ansi_state = STATE_NORMAL;
int ansi_param = 0;


void print(const char* s) {
    while (*s) {
        switch (*s) {
            case '\n':
                newLine();
                break;

            case '\r':
                column = 0;
                break;

            case '\t':
                if (column == width) newLine();
                uint16_t spaces = 4 - (column % 4);
                while (spaces--) {
                    vga[line * width + column++] = ' ' | currentColor;
                }
                break;

            case '\b':
                if (column > 0) column--;
                else if (line > 0) {
                    line--;
                    column = width - 1;
                }
                vga[line * width + column] = ' ' | currentColor;
                break;

            default:
                if (column == width) newLine();
                vga[line * width + column++] = *s | currentColor;
                break;
        }

        s++;
    }
    updateCursor();
}

void scrollUp() {
    for (uint16_t y = 1; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            vga[(y - 1) * width + x] = vga[y * width + x];
        }
    }

    for (uint16_t x = 0; x < width; x++) {
        vga[(height - 1) * width + x] = ' ' | currentColor;
    }

    for (int i = 0; i < 5; i++) {
        putToCoords(75+i, 23, ' ', (color8_Bright_White << 8) | (color8_Black << 12));
    }
}

void scrollUpSpecific(uint8_t y1, uint8_t y2) {
    for (uint16_t y = y1; y <= y2; y++) {
        for (uint16_t x = 0; x < width; x++) {
            vga[(y - 1) * width + x] = vga[y * width + x];
        }
    }
}

void newLine() {
    if (line < height - 1) {
        line++;
        column = 0;
    } else {
        scrollUp();
        column = 0;
    }
}

void updateCursor() {
    uint16_t pos = line * width + column;
    outPortB(0x3D4, 0x0F);
    outPortB(0x3D5, (uint8_t)(pos & 0xFF));
    outPortB(0x3D4, 0x0E);
    outPortB(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void setCursor(uint16_t col, uint16_t row) {
    if (col >= width)  col = width - 1;
    if (row >= height) row = height - 1;
    column = col;
    line   = row;
    updateCursor();
}

void enableCursor(uint8_t start, uint8_t end) {
    outPortB(0x3D4, 0x0A);
    outPortB(0x3D5, (start & 0x1F));   // clear disable bit → cursor on
    outPortB(0x3D4, 0x0B);
    outPortB(0x3D5, end & 0x1F);
}

void disableCursor() {
    outPortB(0x3D4, 0x0A);
    outPortB(0x3D5, 0x20);             // bit 5 set = cursor disabled
}

void Reset() {
    line = 0;
    column = 0;
    currentColor = defaultColor;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            vga[y * width + x] = ' ' | currentColor;
        }
    }
}

void ResetSpecific(uint8_t y1, uint8_t y2) {
    line = 0;
    column = 0;
    currentColor = defaultColor;

    for (uint16_t y = y1; y <= y2; y++) {
        for (uint16_t x = 0; x < width; x++) {
            vga[y * width + x] = ' ' | currentColor;
        }
    }
}

void putToCoords(uint16_t x, uint16_t y, const char c, uint16_t color) {
    vga[y * width + x] = c | color;
}