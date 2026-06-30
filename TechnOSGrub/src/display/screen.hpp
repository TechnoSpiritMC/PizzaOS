#pragma once

extern "C" {
    #include "display.h"

    void drawLine_C(int x1, int y1, int x2, int y2, bool antialias, int width, uint32_t color, int privilege);
    void drawRect_C(int x, int y, int width, int height, uint32_t color, int privilege);
    void drawRectWithBorders_C(int x, int y, int width, int height, int borderWidth, uint32_t color, uint32_t borderColor, int privilege);
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

    void drawPixel_p(int x, int y, Color color, int privilege);
    void drawRect_p(int x, int y, int width, int height, Color color, int privilege);
    void clearScreen_p(int privilege);
    void drawRectWithBorders_p(int x, int y, int width, int height, int borderWidth, Color color, Color borderColor, int privilege);
    void drawLine_p(int x1, int y1, int x2, int y2, bool antialias, int width, Color color, int privilege);

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
    void __drawPixel(int x, int y, float brightness, Color color, int privilege);
    void __drawLine(int x0, int y0, int x1, int y1, float width, Color color, bool antialias, int privilege);
    void __drawAALine(int x0, int y0, int x1, int y1, Color color, int privilege);
}