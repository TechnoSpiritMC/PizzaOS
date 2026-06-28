#include "screen.hpp"

extern "C" {
    #include "display.h"
}

namespace Screen {
    void drawPixel(int x, int y, Color color) {
        draw_pixel(x, y, color.toRGB());
    }

    void drawRect(int x, int y, int width, int height, Color color) {
        for (u16 _x = 0; _x < width; _x++) {
            for (u16 _y = 0; _y < height; _y++) {
                draw_pixel(x + _x, y + _y, color.toRGB());
            }
        }
    }

    void clearScreen() {
        drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color(0));
    }

    void drawRectWithBorders(int x, int y, int width, int height, int borderWidth, Color color, Color borderColor) {
        drawRect(x, y, width, height, borderColor);
        drawRect(x + borderWidth, y + borderWidth, width - borderWidth * 2, height - borderWidth * 2, color);
    }

    void drawLine(int x1, int y1, int x2, int y2, bool antialias, int width, Color color) {
        __drawLine(x1, y1, x2, y2, width, color, antialias);
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

    void __drawPixel(int x, int y, float brightness, Color color) {
        int r = color.r * brightness;
        int g = color.g * brightness;
        int b = color.b * brightness;
        drawPixel(x, y, Color(r, g, b));
    }

    // width: thickness of the line in pixels (1 = original behavior)
    // antialias: true = smooth edges, false = hard-edged solid fill
    void __drawLine(int x0, int y0, int x1, int y1, float width, Color color, bool antialias) {
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
                        __drawPixel(y, x, coverage, color);
                    else
                        __drawPixel(x, y, coverage, color);
                }
            } else {
                int yStart = __roundNumber(center - halfWidth);
                int yEnd   = __roundNumber(center + halfWidth);

                for (int y = yStart; y <= yEnd; y++) {
                    if (steep)
                        drawPixel(y, x, color);
                    else
                        drawPixel(x, y, color);
                }
            }

            intersectY += gradient;
        }
    }

    // Convenience overload to preserve your original call signature/behavior
    void __drawAALine(int x0, int y0, int x1, int y1, Color color) {
        __drawLine(x0, y0, x1, y1, 1.0f, color, true);
    }
}