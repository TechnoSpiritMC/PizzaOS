//
// Created by denis on 6/19/2026.
//

extern "C" {
    #include "include/IApplication.h"
    #include "../include/stdint.h"
    #include "../display/screen.hpp"
    #include "../stdlib/list.hpp"
}

#define BUTTON_STATE_RELEASED 0
#define BUTTON_STATE_HOVERED  1
#define BUTTON_STATE_BLOCKED  2
#define BUTTON_STATE_PRESSED  3
#define BUTTON_STATE_DISABLED 4

using namespace Screen;

static constexpr uint8_t BUTTON_FILL_R[] = {
    229, 163, 221, 147, 107
};
static constexpr uint8_t BUTTON_FILL_G[] = {
    229, 163, 147, 147, 107
};
static constexpr uint8_t BUTTON_FILL_B[] = {
    229, 163, 147, 226, 107
};
static constexpr uint8_t BUTTON_BORDER_R[] = {
    163, 229, 130, 48, 150
};
static constexpr uint8_t BUTTON_BORDER_G[] = {
    163, 229, 48, 48, 150
};
static constexpr uint8_t BUTTON_BORDER_B[] = {
    163, 229, 48, 130, 150
};

#define BUTTON_FILL_COLOR(state) \
Color(BUTTON_FILL_R[state], BUTTON_FILL_G[state], BUTTON_FILL_B[state])

#define BUTTON_BORDER_COLOR(state) \
Color(BUTTON_BORDER_R[state], BUTTON_BORDER_G[state], BUTTON_BORDER_B[state])

namespace Application {
    class IApplication {
    public:
        char* name;
        List::ArrayList* elements;
        virtual void run(uint32_t flags) = 0;

        virtual ~IApplication() {
            List::list_free(elements);
        }

        void initialize() {
            List::list_init(elements);
        }

        IApplication(char* name): name(name) {}
    };

    class DisplayElement {
    public:
        virtual ~DisplayElement() = default;

        bool isWindow = false;
        bool isButton = false;
        bool isText   = false;
        bool isImage  = false;
        bool isConst  = false;

        const uint16_t width  = 0;
        const uint16_t height = 0;
        const uint16_t x      = 0;
        const uint16_t y      = 0;

        virtual void draw(IApplication &application) = 0;

        bool isInside(uint16_t x, uint16_t y) const {
            return x >= this->x && x <= this->x + this->width && y >= this->y && y <= this->y + this->height;
        }
    };

    class Button: public DisplayElement {
    public:
        const uint16_t width;
        const uint16_t height;
        const uint16_t x;
        const uint16_t y;
        const char* text;
        const void* action;

        IApplication& application;

        int state = BUTTON_STATE_DISABLED;

        Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* text, const void* action, const IApplication &application): width(width), height(height), x(x), y(y), text(text), action(action) {
            ((DisplayElement)this).isButton = true;

            ((DisplayElement)this).x      = x;
            ((DisplayElement)this).y      = y;
            ((DisplayElement)this).width  = width;
            ((DisplayElement)this).height = height;

            state = BUTTON_STATE_RELEASED;
            this->application = application;
            Button::draw(application);
        }

        void setBlocked(bool blocked) {
            state = blocked ? BUTTON_STATE_BLOCKED : BUTTON_STATE_RELEASED;
            Button::draw(application);
        }

        void setHovered(bool hovered) {
            state = hovered ? BUTTON_STATE_HOVERED : BUTTON_STATE_RELEASED;
            Button::draw(application);
        }

        void press(const uint8_t mouse_flags) {
            if (state == BUTTON_STATE_BLOCKED) return;
            if (state == BUTTON_STATE_PRESSED) return;

            state =  BUTTON_STATE_PRESSED;
            Button::draw(application);
            ((void (*)(uint8_t mouse_flags))action)(mouse_flags);
            state = BUTTON_STATE_RELEASED;
            Button::draw(application);

        }

        void draw(IApplication& application) override {
            drawRectWithBorders(x, y, width, height, 1, BUTTON_FILL_COLOR(state), BUTTON_BORDER_COLOR(state));
            draw_string(x + 3, y + 3 + font_height(MONOSPACE1), text, Color(0).toRGB(), BUTTON_FILL_COLOR(state).toRGB(), MONOSPACE1);
        }
    };

    class DisplaySection: DisplayElement {
        const uint16_t x;
        const uint16_t y;
        const uint16_t width;
        const uint16_t height;

        List::ArrayList elements;

    public:

        DisplaySection(uint16_t x, uint16_t y, uint16_t width, uint16_t height): x(x), y(y), width(width), height(height) {
            List::list_init(&elements);

            ((DisplayElement)this).x      = x;
            ((DisplayElement)this).y      = y;
            ((DisplayElement)this).width  = width;
            ((DisplayElement)this).height = height;
        }

        void draw(IApplication &application) override {
            for (uint32_t i = 0; i < elements.size; i++) {
                ((DisplayElement*)elements.items[i])->draw(*((IApplication*)elements.items[i]));
            }
        }

        List::ArrayList* getElements() {
            return &elements;
        }
    };

    class Window: public DisplayElement {
        const uint16_t width;
        const uint16_t height;
        const uint16_t x;
        const uint16_t y;

    public:
        Window(uint16_t x, uint16_t y, uint16_t width, uint16_t height, IApplication application): width(width), height(height), x(x), y(y) {
            draw(application);
            ((DisplayElement)this).isWindow = true;

            ((DisplayElement)this).x      = x;
            ((DisplayElement)this).y      = y;
            ((DisplayElement)this).width  = width;
            ((DisplayElement)this).height = height;
        }

        void draw(IApplication application) const {
            drawRectWithBorders(x ,y, width, height, 1, Color(255, 255, 255), Color(192, 192, 200));
            drawRect(x + 1, y + 21, width - 2, height - 22, Color(0, 0, 0));
            drawRect(x+1, y+1, width-2, 20, Color(192, 192, 200));
            draw_string(x + 3, y + 3 + font_height(MONOSPACE1), application.name, Color(0).toRGB(), Color(192, 192, 200).toRGB(), MONOSPACE1);
        }
    };
}

