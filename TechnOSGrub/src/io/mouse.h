#pragma once
#include <stdbool.h>

#include "../include/stdint.h"

static inline bool isRightPressed(uint8_t state) {
    return (state & 0x02) != 0;
}
static inline bool isLeftPressed(uint8_t state) {
    return (state & 0x01) != 0;
}
static inline bool isMiddlePressed(uint8_t state) {
    return (state & 0x04) != 0;
}
