#pragma once

extern "C" {
    #include "screen.h"
    #include <stdbool.h>
    #include "display.h"
}

namespace Screen {
    class Color {
    public:
        uint8_t r, g, b;
        constexpr Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
        constexpr Color() : Color(0, 0, 0) {}
        constexpr Color(uint8_t gray) : Color(gray, gray, gray) {}

        ~Color() = default;

        uint32_t toRGB() const {
            return (r << 16) | (g << 8) | b;
        }
    };

    // Public API Signatures
    void drawPixel(int x, int y, Color color);
    void drawRect(int x, int y, int width, int height, Color color);
    void clearScreen();
    void drawRectWithBorders(int x, int y, int width, int height, int borderWidth, Color color, Color borderColor);
    void drawLine(int x1, int y1, int x2, int y2, bool antialias, int width, Color color);

    // Internal Helper Signatures
    void __swap(int* a, int* b);
    float __abs(float x);
    float __max(float a, float b);
    float __clamp01(float x);
    int __iPartOfNumber(float x);
    int __roundNumber(float x);
    float __fPartOfNumber(float x);
    float __rfPartOfNumber(float x);
    float __sqrt(float x);
    void __drawPixel(int x, int y, float brightness, Color color);
    void __drawLine(int x0, int y0, int x1, int y1, float width, Color color, bool antialias);
    void __drawAALine(int x0, int y0, int x1, int y1, Color color);
}