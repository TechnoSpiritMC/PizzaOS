#include "include/stdint.h"
#include "include/util.h"

void memset(void *dest, char val, uint32_t count){
    char *temp = (char*) dest;
    for (; count != 0; count --){
        *temp++ = val;
    }

}

void outPortB(uint16_t Port, uint8_t Value){
    asm volatile ("outb %1, %0" : : "dN" (Port), "a" (Value));
}

char inPortB(uint16_t Port) {
    char rv;
    asm volatile ("inb %1, %0" : "=a" (rv) : "Nd" (Port));
    return rv;
}

uint32_t strlen(const char *str) {
    uint16_t len = 0;
    while (*str++) len++;
    return len;
}

void strConcat(char *dest, const char *src1, const char *src2) {
    uint32_t lenSrc1 = strlen(src1);
    uint32_t lenSrc2 = strlen(src2);

    for (uint32_t i = 0; i < lenSrc1; i++) {
        dest[i] = src1[i];
    }
    for (uint32_t i = 0; i < lenSrc2; i++) {
        dest[i + lenSrc1] = src2[i];
    }

    dest[lenSrc1 + lenSrc2] = '\0';
}

char* utoa64(uint64_t value, char* buf) {
    static const uint64_t powers[] = {
        10000000000000000000ULL,
        1000000000000000000ULL,
        100000000000000000ULL,
        10000000000000000ULL,
        1000000000000000ULL,
        100000000000000ULL,
        10000000000000ULL,
        1000000000000ULL,
        100000000000ULL,
        10000000000ULL,
        1000000000ULL,
        100000000ULL,
        10000000ULL,
        1000000ULL,
        100000ULL,
        10000ULL,
        1000ULL,
        100ULL,
        10ULL,
        1ULL
    };

    char* p = buf;
    int started = 0;

    for (int i = 0; i < 20; i++) {
        uint64_t count = 0;
        while (value >= powers[i]) {
            value -= powers[i];
            count++;
        }

        if (count || started || i == 19) {
            *p++ = '0' + (char)count;
            started = 1;
        }
    }

    *p = '\0';
    return buf;
}

char* utoa64_hex(uint64_t v, char* buf) {
    static const char hex[] = "0123456789abcdef";
    for (int i = 15; i >= 0; i--) {
        buf[15 - i] = hex[(v >> (i * 4)) & 0xF];
    }
    buf[16] = '\0';
    return buf;
}