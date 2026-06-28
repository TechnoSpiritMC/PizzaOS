#include "include/IApplication.hpp"
#include "../memory/oop.h"

using namespace Screen;

static constexpr uint8_t BUTTON_FILL_R[]   = { 229, 163, 221, 147, 107 };
static constexpr uint8_t BUTTON_FILL_G[]   = { 229, 163, 147, 147, 107 };
static constexpr uint8_t BUTTON_FILL_B[]   = { 229, 163, 147, 226, 107 };
static constexpr uint8_t BUTTON_BORDER_R[] = { 163, 229, 130,  48, 150 };
static constexpr uint8_t BUTTON_BORDER_G[] = { 163, 229,  48,  48, 150 };
static constexpr uint8_t BUTTON_BORDER_B[] = { 163, 229,  48, 130, 150 };

#define BUTTON_FILL_COLOR(state) \
    Color(BUTTON_FILL_R[state], BUTTON_FILL_G[state], BUTTON_FILL_B[state])

#define BUTTON_BORDER_COLOR(state) \
    Color(BUTTON_BORDER_R[state], BUTTON_BORDER_G[state], BUTTON_BORDER_B[state])

namespace Application {

    // ==========================================
    // BUTTON IMPLEMENTATION
    // ==========================================
    Button::Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                   const char* text, const void* action, IApplication &application)
        : DisplayElement(x, y, width, height), text(text), action(action), application(application), state(BUTTON_STATE_RELEASED)
    {
        this->isButton = true;
        Button::draw(application);
    }

    void Button::setBlocked(bool blocked) {
        state = blocked ? BUTTON_STATE_BLOCKED : BUTTON_STATE_RELEASED;
        Button::draw(application);
    }

    void Button::setHovered(bool hovered) {
        state = hovered ? BUTTON_STATE_HOVERED : BUTTON_STATE_RELEASED;
        Button::draw(application);
    }

    void Button::press(const uint8_t mouse_flags) {
        if (state == BUTTON_STATE_BLOCKED) return;
        if (state == BUTTON_STATE_PRESSED) return;

        state = BUTTON_STATE_PRESSED;
        Button::draw(application);

        if (action) {
            ((void (*)(uint8_t))action)(mouse_flags);
        }

        state = BUTTON_STATE_RELEASED;
        Button::draw(application);
    }

    void Button::draw(IApplication& _application) {
        (void)_application; // Suppress unused parameter warning cleanly
        drawRectWithBorders(x, y, width, height, 1, BUTTON_FILL_COLOR(state), BUTTON_BORDER_COLOR(state));
        draw_string(x + 3, y + 3 + font_height(MONOSPACE1), text, Color(0).toRGB(), BUTTON_FILL_COLOR(state).toRGB(), MONOSPACE1);
    }

    void Button::redrawWhenInside(uint16_t target_x, uint16_t target_y) { // Removed override keyword
        if (isInside(target_x, target_y)) {
            Button::draw(application);
        }
    }

    // ==========================================
    // DISPLAYSECTION IMPLEMENTATION
    // ==========================================
    DisplaySection::DisplaySection(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
        : DisplayElement(x, y, width, height)
    {
        List::list_init(&elements); // Fixed namespace reference setup
    }

    void DisplaySection::draw(IApplication &application) { // Removed override keyword
        for (uint32_t i = 0; i < elements.size; i++) {
            ((DisplayElement*)elements.items[i])->draw(application);
        }
    }

    void DisplaySection::redrawWhenInside(uint16_t target_x, uint16_t target_y) { // Removed override keyword
        for (uint32_t i = 0; i < elements.size; i++) {
            DisplayElement* element = (DisplayElement*)elements.items[i];
            if (element->isInside(target_x, target_y)) {
                element->redrawWhenInside(target_x, target_y);
            }
        }
    }

    List::ArrayList* DisplaySection::getElements() {
        return &elements;
    }

    // ==========================================
    // WINDOW IMPLEMENTATION
    // ==========================================
    Window::Window(uint16_t x, uint16_t y, uint16_t width, uint16_t height, IApplication &application)
        : DisplayElement(x, y, width, height), application(application)
    {
        this->isWindow = true;
        Window::draw(application);
    }

    void Window::draw(IApplication& _application) { // Removed override keyword
        (void)_application; // Suppress unused parameter warning cleanly
        drawRectWithBorders(x, y, width, height, 1, Color(255, 255, 255), Color(192, 192, 200));
        drawRect(x + 1, y + 21, width - 2, height - 22, Color(0, 0, 0));
        drawRect(x + 1, y + 1, width - 2, 20, Color(192, 192, 200));
        draw_string(x + 3, y + 3 + font_height(MONOSPACE1), application.name, Color(0).toRGB(), Color(192, 192, 200).toRGB(), MONOSPACE1);
    }

    void Window::redrawWhenInside(uint16_t target_x, uint16_t target_y) { // Removed override keyword
        if (isInside(target_x, target_y)) {
            Window::draw(application);
        }
    }

    void Window::close() {
        drawRect(x, y, width, height, Color(0));
    }

    // ==========================================
    // IAPPLICATION IMPLEMENTATION
    // ==========================================
    IApplication::IApplication(char* name)
        : name(name), elements(nullptr), window(nullptr) {}

    IApplication::~IApplication() {
        if (elements) {
            List::list_free(elements);
        }
        if (window) {
            delete window;
        }
    }

    void IApplication::run(uint32_t flags) {
        List::list_init(elements);

        window = new Window(0, 0, 320, 240, *this);
        mouse_add_listener((void*)this);

        main(flags);

        window->close();
    }

    void IApplication::onMouseEvent(uint8_t mouse_flags) {
        if (!window || !window->isInside(__mx, __my)) return;

        userMouseEventHandler(__mx, __my, mouse_flags);
        onMouseMove();
    }

    void IApplication::onMouseMove() {
        if (!elements) return;

        for (uint32_t i = 0; i < elements->size; i++) {
            ((DisplayElement*)elements->items[i])->redrawWhenInside(__mx, __my);
        }
    }
}