#include "include/SampleApp.hpp"

extern "C" {
    #include "../include/stdint.h"
    #include "../io/mouse.h"
    #include "../stdlib/serial.h"
}

extern "C" void start_sample_app() {
    serial_printf("Creating app instance.\r\n");
    Application::DemoApp app = Application::DemoApp();
    serial_printf("App instance created. Running it...\r\n");
    app.run(0);
}

bool running = true;

void onClickMePressed(uint8_t flags) {
    (void)flags;
}

void onExitPressed(uint8_t flags) {
    (void)flags;
    LOG_LINE();
    running = false;
}

namespace Application {

    void DemoApp::main(uint32_t flags) {
        LOG_LINE();
        (void)flags;

        running = true;

        clickMeButton = new Button(20, 40, 100, 30, "CLICK ME", (const void*)onClickMePressed, *this);
        exitButton    = new Button(130, 40, 100, 30, "EXIT APP", (const void*)onExitPressed, *this);

        exitButton->setBlocked(true);

        List::list_append(this->elements, (void*)clickMeButton);
        List::list_append(this->elements, (void*)exitButton);

        this->onMouseMove();

        serial_printf("App is running. Waiting for user interaction... Running is %d\r\n", running);
        while (running) {
            serial_printf("Running is %d\r", running);
        }
        window->close();
    }

    void DemoApp::userMouseEventHandler(uint16_t mouse_x, uint16_t mouse_y, uint8_t mouse_flags) {
        if (clickMeButton->isInside(mouse_x, mouse_y)) {
            LOG_LINE();
            clickMeButton->setHovered(true);

            if (isLeftPressed(mouse_flags)) {
                LOG_LINE();
                serial_printf("Click Me button pressed. Executing action...\r\n");
                clickMeButton->press(mouse_flags);
            }
        } else {
            LOG_LINE();
            clickMeButton->setHovered(false);
        }

        if (exitButton->isInside(mouse_x, mouse_y)) {
            LOG_LINE();
            exitButton->setHovered(true);
            if (isLeftPressed(mouse_flags)) {
                LOG_LINE();
                serial_printf("Exit button pressed. Exiting app...\r\n");
                exitButton->press(mouse_flags);
            }
        } else {
            LOG_LINE();
            exitButton->setHovered(false);
        }
    }

    void DemoApp::redrawAll() {
        app_redraw_all_elements();
    }

    DemoApp::~DemoApp() {
        serial_printf("Destroying app instance.\r\n");
        if (clickMeButton) delete clickMeButton;
        if (exitButton) delete exitButton;
        serial_printf("App instance destroyed.\r\n");
    }
}