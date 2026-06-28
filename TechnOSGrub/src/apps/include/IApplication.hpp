#pragma once


extern "C" {
    #include "../../io/mouse.h"
    #include "../../include/stdint.h"

    // Explicitly expose mouse listeners if not present in mouse.h
    uint8_t mouse_add_listener(void* listener);
}

#include "../../display/screen.hpp"
#include "../../stdlib/list.hpp"

#define BUTTON_STATE_RELEASED 0
#define BUTTON_STATE_HOVERED  1
#define BUTTON_STATE_BLOCKED  2
#define BUTTON_STATE_PRESSED  3
#define BUTTON_STATE_DISABLED 4

namespace Application {

    class IApplication;

    class DisplayElement {
    public:
        virtual ~DisplayElement() = default;

        bool isWindow = false;
        bool isButton = false;
        bool isText   = false;
        bool isImage  = false;
        bool isConst  = false;

        uint16_t width  = 0;
        uint16_t height = 0;
        uint16_t x      = 0;
        uint16_t y      = 0;

        DisplayElement() = default;
        DisplayElement(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
            : width(width), height(height), x(x), y(y) {}

        virtual void draw(IApplication &application) = 0;
        virtual void redrawWhenInside(uint16_t x, uint16_t y) = 0;

        bool isInside(uint16_t x, uint16_t y) const {
            return x >= this->x && x <= this->x + this->width &&
                   y >= this->y && y <= this->y + this->height;
        }
    };

    class Button : public DisplayElement {
    public:
        const char* text;
        const void* action;
        IApplication& application;
        int state = BUTTON_STATE_DISABLED;

        Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
               const char* text, const void* action, IApplication &application);

        void setBlocked(bool blocked);
        void setHovered(bool hovered);
        void press(const uint8_t mouse_flags);

        void draw(IApplication& _application) override;
        void redrawWhenInside(uint16_t x, uint16_t y) override;
    };

    class DisplaySection : public DisplayElement {
    private:
        List::ArrayList elements; // Fixed namespace token reference
    public:
        DisplaySection(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

        void draw(IApplication &application) override;
        void redrawWhenInside(uint16_t x, uint16_t y) override;
        List::ArrayList* getElements();
    };

    class Window : public DisplayElement {
    private:
        IApplication& application;
    public:
        Window(uint16_t x, uint16_t y, uint16_t width, uint16_t height, IApplication &application);

        void draw(IApplication& _application) override;
        void redrawWhenInside(uint16_t x, uint16_t y) override;
        void close();
    };

    class IApplication {
    public:
        char* name;
        List::ArrayList* elements; // Fixed namespace token reference
        Window* window;

        IApplication(char* name);
        virtual ~IApplication();

        virtual void main(uint32_t flags) = 0;
        virtual void userMouseEventHandler(uint16_t x, uint16_t y, uint8_t mouse_flags) = 0;

        void run(uint32_t flags);
        void onMouseEvent(uint8_t mouse_flags);
        void onMouseMove();
    };
}