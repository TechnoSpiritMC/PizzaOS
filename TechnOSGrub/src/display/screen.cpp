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

        uint32_t toRGB() {
            return (r << 16) | (g << 8) | b;
        }
    };

    static void drawPixel(int x, int y, Color color) {
        draw_pixel(x, y, color.toRGB());
    }

    static void drawRect(int x, int y, int width, int height, Color color) {

    }

    static void clearScreen() {
        Screen::drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color(0));
    }
    static void drawRectWithBorders(int x, int y, int width, int height, int borderWidth, Color color, Color borderColor) {

    }

    static void drawLine(int x1, int y1, int x2, int y2, bool antialias, int width, Color color) {

    }
}