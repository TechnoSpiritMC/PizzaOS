#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef char* string_t;

inline bool strcmp(const char* str1, const char* str2) {
    uint32_t idx = 0;

    while (str1[idx] != '\0' && str2[idx] != '\0') {
        if (str1[idx] != str2[idx]) return false;
        idx++;
    }
    return str1[idx] == '\0' && str2[idx] == '\0';
}