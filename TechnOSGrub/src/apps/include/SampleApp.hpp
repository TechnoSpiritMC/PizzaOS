#pragma once
#include "IApplication.hpp"

namespace Application {

    class DemoApp : public IApplication {
    private:
        // We declare the buttons that will live in this application
        Button* clickMeButton;
        Button* exitButton;

    public:
        // Pass the app name up to the base IApplication constructor
        DemoApp() : IApplication((char*)"DEMO PANEL") {}
        ~DemoApp() override;

        // Implement the mandatory lifecycle hooks
        void main(uint32_t flags) override;
        void userMouseEventHandler(uint16_t x, uint16_t y, uint8_t mouse_flags) override;
        void redrawAll();
    };

}