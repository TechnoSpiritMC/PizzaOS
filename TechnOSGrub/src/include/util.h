#pragma once
#include "stdint.h"

#define debug 0
#define advancedLogging 1
#define newConsole 1

#define CEIL_DIV(a, b) (((a + b) - 1) / b)


void memset(void *dest, char val, uint32_t count);
void outPortB(uint16_t port, uint8_t val);
char inPortB(uint16_t Port);
uint32_t strlen(const char *str);
void strConcat(char *dest, const char *src1, const char *src2);
char* utoa64_hex(uint64_t v, char* buf);
char* utoa64(uint64_t v, char* buf);

struct InterruptRegisters {
    uint32_t cr2;
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};