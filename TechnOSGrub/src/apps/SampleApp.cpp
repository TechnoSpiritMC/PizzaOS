#include "include/SampleApp.hpp"

extern "C" {
    #include "../include/stdint.h"
    #include "../io/mouse.h"
}

extern "C" void start_sample_app() {
    // Inside a C++ file, this syntax is completely valid
    Application::DemoApp app;
    app.run(0);
}

static bool running = true;

// Example callback functions matching your "const void* action" signature
void onClickMePressed(uint8_t flags) {
    (void)flags;
    // Do something inside your OS kernel when the button clicks!
}

void onExitPressed(uint8_t flags) {
    (void)flags;
    running = false;
}

namespace Application {

    void DemoApp::main(uint32_t flags) {
        (void)flags;

        // 1. Instantiate the UI buttons inside the window dimensions
        // Window is 320x240, positioned at 0,0 by default in your framework
        clickMeButton = new Button(20, 40, 100, 30, "Click Me", (const void*)onClickMePressed, *this);
        exitButton    = new Button(130, 40, 100, 30, "Exit App", (const void*)onExitPressed, *this);

        // 2. Register these components into the base class elements list
        // so the system handles refreshing/drawing them automatically
        List::list_append(this->elements, (void*)clickMeButton);
        List::list_append(this->elements, (void*)exitButton);

        // 3. Force initial rendering of all elements inside the framework loop
        this->onMouseMove();

        while (running) {}

        window->close();
        delete this;
    }

    void DemoApp::userMouseEventHandler(uint16_t mouse_x, uint16_t mouse_y, uint8_t mouse_flags) {
        // This receives raw coordinates whenever the mouse moves or clicks inside the window boundaries.

        // Check if the user is hovering over or pressing our components
        if (clickMeButton->isInside(mouse_x, mouse_y)) {
            clickMeButton->setHovered(true);

            if (isLeftPressed(mouse_flags)) {
                clickMeButton->press(mouse_flags);
            }
        } else {
            clickMeButton->setHovered(false);
        }

        if (exitButton->isInside(mouse_x, mouse_y)) {
            exitButton->setHovered(true);
            if (isLeftPressed(mouse_flags)) {
                exitButton->press(mouse_flags);
            }
        } else {
            exitButton->setHovered(false);
        }
    }

    DemoApp::~DemoApp() {
        if (clickMeButton) delete clickMeButton;
        if (exitButton) delete exitButton;
    }
}