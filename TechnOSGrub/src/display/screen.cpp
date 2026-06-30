#include "screen.hpp"

extern "C" {
    #include "display.h"

    void drawLine_C(int x1, int y1, int x2, int y2, bool antialias, int width, uint32_t color, int privilege) {
        Screen::__drawLine(x1, y1, x2, y2, width, Screen::Color(color>>16 & 0xFF, color>>8 & 0xFF, color & 0xFF), antialias, privilege);
    }
    void drawRect_C(int x, int y, int width, int height, uint32_t color, int privilege) {
        Screen::drawRect_p(x, y, width, height, Screen::Color(color>>16 & 0xFF, color>>8 & 0xFF, color & 0xFF), privilege);
    }
    void drawRectWithBorders_C(int x, int y, int width, int height, int borderWidth, uint32_t color, uint32_t borderColor, int privilege) {
        Screen::drawRectWithBorders_p(x, y, width, height, borderWidth, Screen::Color(color>>16 & 0xFF, color>>8 & 0xFF, color & 0xFF), Screen::Color(borderColor>>16 & 0xFF, borderColor>>8 & 0xFF, borderColor & 0xFF), privilege);
    }
}

namespace Screen {
    void drawPixel(int x, int y, Color color) {
        draw_pixel(x, y, color.toRGB(), DISPLAY_PRIVILEDGE_LOW);
    }
    void drawPixel_p(int x, int y, Color color, int privilege) {
        draw_pixel(x, y, color.toRGB(), privilege);
    }

    void drawRect(int x, int y, int width, int height, Color color) {
        for (u16 _x = 0; _x < width; _x++) {
            for (u16 _y = 0; _y < height; _y++) {
                draw_pixel(x + _x, y + _y, color.toRGB(), DISPLAY_PRIVILEDGE_LOW);
            }
        }
    }
    void drawRect_p(int x, int y, int width, int height, Color color, int privilege) {
        for (u16 _x = 0; _x < width; _x++) {
            for (u16 _y = 0; _y < height; _y++) {
                draw_pixel(x + _x, y + _y, color.toRGB(), privilege);
            }
        }
    }

    void clearScreen() {
        drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color(0));
    }
    void clearScreen_p(int privilege) {
        drawRect_p(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color(0), privilege);
    }

    void drawRectWithBorders(int x, int y, int width, int height, int borderWidth, Color color, Color borderColor) {
        drawRect(x, y, width, height, borderColor);
        drawRect(x + borderWidth, y + borderWidth, width - borderWidth * 2, height - borderWidth * 2, color);
    }
    void drawRectWithBorders_p(int x, int y, int width, int height, int borderWidth, Color color, Color borderColor, int privilege) {
        drawRect_p(x, y, width, height, borderColor, privilege);
        drawRect_p(x + borderWidth, y + borderWidth, width - borderWidth * 2, height - borderWidth * 2, color, privilege);
    }

    void drawLine(int x1, int y1, int x2, int y2, bool antialias, int width, Color color) {
        __drawLine(x1, y1, x2, y2, width, color, antialias, DISPLAY_PRIVILEDGE_LOW);
    }
    void drawLine_p(int x1, int y1, int x2, int y2, bool antialias, int width, Color color, int privilege) {
        __drawLine(x1, y1, x2, y2, width, color, antialias, privilege);
    }

    void __swap(int* a, int* b) {
        int temp = *a;
        *a = *b;
        *b = temp;
    }

    float __abs(float x) {
        if (x < 0) return -x;
        return x;
    }

    float __max(float a, float b) {
        return a > b ? a : b;
    }

    float __clamp01(float x) {
        if (x < 0) return 0;
        if (x > 1) return 1;
        return x;
    }

    int __iPartOfNumber(float x) {
        return (int)x;
    }

    int __roundNumber(float x) {
        return __iPartOfNumber(x + 0.5f);
    }

    float __fPartOfNumber(float x) {
        if (x > 0) return x - __iPartOfNumber(x);
        return x - (__iPartOfNumber(x) + 1);
    }

    float __rfPartOfNumber(float x) {
        return 1 - __fPartOfNumber(x);
    }

    // Freestanding sqrt via Newton-Raphson (no std lib available)
    float __sqrt(float x) {
        if (x <= 0) return 0;
        float guess = x;
        for (int i = 0; i < 16; i++) {
            guess = 0.5f * (guess + x / guess);
        }
        return guess;
    }

    void __drawPixel(int x, int y, float brightness, Color color, int privilege) {
        int r = color.r * brightness;
        int g = color.g * brightness;
        int b = color.b * brightness;
        drawPixel_p(x, y, Color(r, g, b), privilege);
    }

    // width: thickness of the line in pixels (1 = original behavior)
    // antialias: true = smooth edges, false = hard-edged solid fill
    void __drawLine(int x0, int y0, int x1, int y1, float width, Color color, bool antialias, int privilege) {
        int steep = __abs(y1 - y0) > __abs(x1 - x0);

        if (steep) {
            __swap(&x0, &y0);
            __swap(&x1, &y1);
        }
        if (x0 > x1) {
            __swap(&x0, &x1);
            __swap(&y0, &y1);
        }

        float dx = x1 - x0;
        float dy = y1 - y0;
        float gradient = dy / dx;
        if (dx == 0.0f)
            gradient = 1;

        if (width < 1.0f) width = 1.0f;

        // Correct the half-width so it represents true perpendicular thickness
        // to the line, not just thickness measured along the minor axis.
        float halfWidth = (width * 0.5f) / __sqrt(1.0f + gradient * gradient);

        int xpxl1 = x0;
        int xpxl2 = x1;
        float intersectY = y0;

        for (int x = xpxl1; x <= xpxl2; x++) {
            float center = intersectY;

            if (antialias) {
                int yStart = __iPartOfNumber(center - halfWidth) - 1;
                int yEnd   = __iPartOfNumber(center + halfWidth) + 1;

                for (int y = yStart; y <= yEnd; y++) {
                    float dist = (float)y - center;
                    float coverage = __clamp01(1.0f - __max(0.0f, __abs(dist) - halfWidth));
                    if (coverage <= 0.0f) continue;

                    if (steep)
                        __drawPixel(y, x, coverage, color, privilege);
                    else
                        __drawPixel(x, y, coverage, color, privilege);
                }
            } else {
                int yStart = __roundNumber(center - halfWidth);
                int yEnd   = __roundNumber(center + halfWidth);

                for (int y = yStart; y <= yEnd; y++) {
                    if (steep)
                        __drawPixel(y, x, 1.0f, color, privilege);
                    else
                        __drawPixel(x, y, 1.0f, color, privilege);
                }
            }

            intersectY += gradient;
        }
    }

    // Convenience overload to preserve your original call signature/behavior
    void __drawAALine_p(int x0, int y0, int x1, int y1, Color color, int privilege) {
        __drawLine(x0, y0, x1, y1, 1.0f, color, true, privilege);
    }
}