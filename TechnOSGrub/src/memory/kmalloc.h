#pragma once

#include "../include/stdint.h"
#include "../include/stdbool.h"

void kmallocInit(uint32_t initialHeapSize);
void changeHeapSize(int newHeapSize);