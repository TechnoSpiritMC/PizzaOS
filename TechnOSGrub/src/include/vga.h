#pragma once
#include "stdint.h"
#include "stdbool.h"

#define color8_Black 0x00
#define color8_Blue 0x01
#define color8_Green 0x02
#define color8_Cyan 0x03
#define color8_Red 0x04
#define color8_Magenta 0x05
#define color8_Brown 0x06
#define color8_White 0x07
#define color8_Gray 0x08
#define color8_Light_Blue 0x09
#define color8_Light_Green 0x0A
#define color8_Light_Cyan 0x0B
#define color8_Light_Red 0x0C
#define color8_Light_Magenta 0x0D
#define color8_Yellow 0x0E
#define color8_Bright_White 0x0F

#define PRINTF_STATE_START        0
#define PRINTF_STATE_LENGTH       1
#define PRINTF_STATE_SHORT        2
#define PRINTF_STATE_LONG         3
#define PRINTF_STATE_SPEC         4
#define PRINTF_STATE_PRECISION    5

#define PRINTF_LENGTH_START       0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT       2
#define PRINTF_LENGTH_LONG        3
#define PRINTF_LENGTH_LONG_LONG   4


#define width 80
#define height 25

extern unsigned long long __udivdi3(unsigned long long numerator, unsigned long long denominator);
extern unsigned long long __umoddi3(unsigned long long numerator, unsigned long long denominator);

void print(const char* s);

void scrollUp();
void scrollUpSpecific(uint8_t y1, uint8_t y2);

void newLine();

void Reset();
void ResetSpecific(uint8_t y1, uint8_t y2);

void updateCursor();
void setCursor(uint16_t col, uint16_t row);
void enableCursor(uint8_t start, uint8_t end);
void disableCursor();

void putToCoords(uint16_t x, uint16_t y, char c, uint16_t color);

// void printf(const char* format, ...);
// int* printf_number(int* argp, int length, bool sign, int radix);