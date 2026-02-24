#pragma once
#include "../include/util.h"

void initTimer();
void onIrq0(struct InterruptRegisters* regs);
void sleep(uint64_t ms);
uint32_t getTicks();
uint32_t getFreq();