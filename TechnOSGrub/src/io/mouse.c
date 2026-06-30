//
// Created by denis on 6/21/2026.
//

#include "mouse.h"

#include "../include/stdint.h"
#include "../include/util.h"
#include "../interrupts/idt.h"
#include "../display/display.h"
#include "../memory/malloc.h"
#include "../stdlib/serial.h"
#include "../timer/timer.h"

void drawLine_C(int x1, int y1, int x2, int y2, bool antialias, int width, uint32_t color, int privilege);
void drawRect_C(int x, int y, int width, int height, uint32_t color, int privilege);
void drawRectWithBorders_C(int x, int y, int width, int height, int borderWidth, uint32_t color, uint32_t borderColor, int privilege);

static volatile bool isCalibrating = false;

static uint16_t p_mouse_x, p_mouse_y = 0;
static uint32_t overridenPixelColor = 0x00000022;

static inline uint32_t saveInterruptStateAndDisable() {
    uint32_t flags;
    asm volatile("pushf; pop %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

static inline void restoreInterruptState(uint32_t flags) {
    if (flags & (1u << 9)) {
        asm volatile("sti" ::: "memory");
    }
}

static void flushMouseControllerOutputBuffer() {
    while (inPortB(0x64) & 0x01) {
        (void)inPortB(0x60);
    }
}

void onMouseMoved(uint8_t flags) {
    getPixelColor(CLAMP(__mx, 0, SCREEN_WIDTH-1), CLAMP(__my, 0, SCREEN_HEIGHT-1), &overridenPixelColor);
    draw_pixel(CLAMP(__mx, 0, SCREEN_WIDTH-1), CLAMP(__my, 0, SCREEN_HEIGHT-1), 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);

    if (p_mouse_x != __mx || p_mouse_y != __my) {
        draw_pixel(CLAMP(p_mouse_x, 0, SCREEN_WIDTH-1), CLAMP(p_mouse_y, 0, SCREEN_HEIGHT-1), overridenPixelColor, DISPLAY_PRIVILEDGE_HIGH);
    }

    p_mouse_x = __mx;
    p_mouse_y =__my;
}

void init_mouse() {
    uint8_t status;
    uint32_t interruptFlags = saveInterruptStateAndDisable();

    LOG_LINE();
    flushMouseControllerOutputBuffer();

    // --- STEP 1: Enable the Mouse Auxiliary Device Port ---
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xA8); // Enable Auxiliary (Mouse) Port

    LOG_LINE();
    // --- STEP 2: Enable IRQ12 in the Controller Config Byte ---
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0x20); // Command 0x20: Read Controller Command Byte

    LOG_LINE();
    while ((inPortB(0x64) & 0x01) == 0) {}
    status = inPortB(0x60); // Read the current configuration byte

    LOG_LINE();
    status |= 0x02;  // Set Bit 1: Enable IRQ 12 for mouse
    status &= ~0x20; // Clear Bit 5: Disable Mouse Clock Disable (meaning enable clock)

    LOG_LINE();
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0x60); // Command 0x60: Write Controller Command Byte

    LOG_LINE();
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, status); // Send updated status byte

    LOG_LINE();
    // --- STEP 3: Tell the mouse itself to reset and start streaming data ---
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xD4);
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, 0xF6); // Set defaults
    while ((inPortB(0x64) & 0x01) == 0) {}
    char ack1 = inPortB(0x60);

    LOG_LINE();
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x64, 0xD4);
    while (inPortB(0x64) & 0x02) {}
    outPortB(0x60, 0xF4); // Enable data reporting
    while ((inPortB(0x64) & 0x01) == 0) {}
    char ack2 = inPortB(0x60);

    LOG_LINE();
    serial_printf("Installed mouse at irq 12\r\n");
    irq_install_handler(12, &mouseHandler);

    LOG_LINE();
    mouse_add_listener((void*)onMouseMoved);

    restoreInterruptState(interruptFlags);
}

uint8_t mouse_packet[3];
uint8_t mouse_packet_size = 0;

int32_t __mx, __my = 0;

void* listeners[64];
uint8_t listener_count = 0;

uint8_t mouse_add_listener(void* listener) {
    if (listener_count >= 64) {
        serial_printf("Too many listeners\r\n");
        return 0;
    }
    listeners[listener_count] = listener;

    serial_printf("Added listener %x. Now got: %x\r\n", listener, listener_count+1);

    return listener_count++;
}

void mouse_remove_listener(uint8_t listener) {
    listeners[listener] = 0;
    for (uint8_t i = listener; i < listener_count; i++) {
        listeners[i] = listeners[i + 1];
    }
    listener_count--;
}

void mouse_notify_listeners(uint8_t flags) {
    for (uint8_t i = 0; i < listener_count; i++) {
        ((void (*)(uint8_t flags))listeners[i])(flags);
    }
}

void mouseHandler(struct InterruptRegisters *r) {
    (void)r;

    uint8_t status = inPortB(0x64);

    if ((status & 0x01) == 0 || (status & 0x20) == 0) {
        return;
    }

    mouse_packet[mouse_packet_size++] = inPortB(0x60);

    if (mouse_packet_size < 3) return;

    if ((mouse_packet[0] & 0x08) == 0) {
        mouse_packet_size = 0;
        return;
    }

    int8_t x = (int8_t)mouse_packet[1];
    int8_t y = (int8_t)mouse_packet[2];

    if (!isCalibrating) {
        __mx += x;
        __my += -y;
    }

    __mx = CLAMP(__mx, 0, SCREEN_WIDTH-1);
    __my = CLAMP(__my, 0, SCREEN_HEIGHT-1);


    mouse_notify_listeners(mouse_packet[0] & 0b111);

    mouse_packet_size = 0;
}

void calibrateMouse() {
    if (isCalibrating) {
        serial_printf("Mouse calibration is already in progress.\r\n");
        return;
    }

    isCalibrating = true;
    setRequiredPrivilege(DISPLAY_PRIVILEDGE_HIGH);

    drawRect_C(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000022, DISPLAY_PRIVILEDGE_HIGH);
    drawRectWithBorders_C(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 150, 400, 300, 2, 0x00000000, 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);
    
    draw_string(SCREEN_WIDTH / 2 - 195, SCREEN_HEIGHT / 2 - 140, "Mouse calibration started.", 0x00ffffff, 0x00000000, MONOSPACE2_BIGGER, DISPLAY_PRIVILEDGE_HIGH);
    draw_string(SCREEN_WIDTH / 2 - 195, SCREEN_HEIGHT / 2 - 140+FONT_MONOSPACE2_HEIGHT, "The virtual mouse pointer was moved to the center of the screen.\r\n", 0x00ffffff, 0x00000000, MONOSPACE1, DISPLAY_PRIVILEDGE_HIGH);
    draw_string(SCREEN_WIDTH / 2 - 195, SCREEN_HEIGHT / 2 - 140+FONT_MONOSPACE2_HEIGHT + FONT_MONOSPACE1_HEIGHT, "Align the virtual pointer with QEMU's. Ctrl+Enter when done.", 0x00ffffff, 0x00000000, MONOSPACE1, DISPLAY_PRIVILEDGE_HIGH);

    __mx = SCREEN_WIDTH / 2;
    __my = SCREEN_HEIGHT / 2;

    drawLine_C(__mx, __my-25, __mx, __my-10, false, 1, 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);
    drawLine_C(__mx, __my+10, __mx, __my+25, false, 1, 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);
    drawLine_C(__mx-25, __my, __mx-10, __my, false, 1, 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);
    drawLine_C(__mx+10, __my, __mx+25, __my, false, 1, 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);

    // Do not block here: this function is triggered from keyboard IRQ context.
    // Returning immediately allows EOI to be sent so Ctrl+Enter can be handled.
}

void completeMouseCalibration() {
    if (!isCalibrating) {
        serial_printf("Mouse calibration is not in progress.\r\n");
        return;
    }

    isCalibrating = false;
    drawRectWithBorders_C(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 150, 400, 300, 2, 0x00000000, 0x00ffffff, DISPLAY_PRIVILEDGE_HIGH);
    draw_string(SCREEN_WIDTH / 2 - 195, SCREEN_HEIGHT / 2 - 140, "Mouse calibration completed.", 0x00ffffff, 0x00000000, MONOSPACE2_BIGGER, DISPLAY_PRIVILEDGE_HIGH);

    drawLine_C(__mx, __my-25, __mx, __my-10, false, 1, 0x00000022, DISPLAY_PRIVILEDGE_HIGH);
    drawLine_C(__mx, __my+10, __mx, __my+25, false, 1, 0x00000022, DISPLAY_PRIVILEDGE_HIGH);
    drawLine_C(__mx-25, __my, __mx-10, __my, false, 1, 0x00000022, DISPLAY_PRIVILEDGE_HIGH);
    drawLine_C(__mx+10, __my, __mx+25, __my, false, 1, 0x00000022, DISPLAY_PRIVILEDGE_HIGH);

    drawRect_C(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000022, DISPLAY_PRIVILEDGE_HIGH);
    setRequiredPrivilege(DISPLAY_PRIVILEDGE_LOW);

    mouse_notify_listeners(0x08);
}