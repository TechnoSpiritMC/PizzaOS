#include "include/SampleApp.hpp"

extern "C" {
    #include "../include/stdint.h"
    #include "../io/mouse.h"
    #include "../stdlib/serial.h"
}

extern "C" void start_sample_app() {
    // Inside a C++ file, this syntax is completely valid
    serial_printf("Creating app instance.\r\n");
    Application::DemoApp app = Application::DemoApp();
    serial_printf("App instance created. Running it...\r\n");
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
        LOG_LINE();
        (void)flags;

        LOG_LINE();
        // 1. Instantiate the UI buttons inside the window dimensions
        // Window is 320x240, positioned at 0,0 by default in your framework
        clickMeButton = new Button(20, 40, 100, 30, "CLICK ME", (const void*)onClickMePressed, *this);
        LOG_LINE();
        exitButton    = new Button(130, 40, 100, 30, "EXIT APP", (const void*)onExitPressed, *this);

        // 2. Register these components into the base class elements list
        // so the system handles refreshing/drawing them automatically
        LOG_LINE();
        List::list_append(this->elements, (void*)clickMeButton);
        LOG_LINE();
        List::list_append(this->elements, (void*)exitButton);

        // 3. Force initial rendering of all elements inside the framework loop
        LOG_LINE();
        this->onMouseMove();

        LOG_LINE();
        while (true) {}

        LOG_LINE();
        window->close();
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