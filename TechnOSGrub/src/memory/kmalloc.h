#pragma once

#include "../include/stdint.h"
#include "../include/stdbool.h"

extern uint32_t heapStart;
extern uint32_t heapSize;
extern uint32_t threshold;
extern bool kmallocInitialized;

void kmallocInit(uint32_t initialHeapSize);
void changeHeapSize(int newHeapSize);