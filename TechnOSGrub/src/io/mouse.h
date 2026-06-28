#pragma once
#include <stdbool.h>

#include "../include/stdint.h"
#include "../include/util.h"

extern uint16_t __mx, __my;

static inline bool isRightPressed(uint8_t state) {
    return (state & 0x02) != 0;
}
static inline bool isLeftPressed(uint8_t state) {
    return (state & 0x01) != 0;
}
static inline bool isMiddlePressed(uint8_t state) {
    return (state & 0x04) != 0;
}

uint8_t mouse_add_listener(void* listener);
void mouse_remove_listener(uint8_t listener);
void mouse_notify_listeners(uint8_t flags);
void mouseHandler(struct InterruptRegisters *r);
void init_mouse();