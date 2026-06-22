//
// Created by denis on 6/19/2026.
//

#include "include/IApplication.h"
#include "../include/stdint.h"
#include "../display/screen.hpp"

using namespace Screen;

namespace Application {
    class IApplication {
    public:
        virtual void run(uint32_t flags) = 0;
    };

    class Window {
        uint16_t width;
        uint16_t height;
        uint16_t x;
        uint16_t y;

        void draw() {
            drawRectWithBorders(x ,y, width, height, 1, Color(255, 255, 255), Color(192, 192, 192));
            drawRect(x + 1, y + 21, width - 2, height - 22, Color(0, 0, 0));
            drawRect(x+1, y+1, width-2, 20, Color(192, 192, 192));
        }
    };
}

