#pragma once

#include "../include/stdint.h"

#define sizeThreshold 8

struct malloc_block_struct {
    char flags[3];
    char used;
    uint32_t size;
    struct malloc_block_struct* next;
} __attribute__((packed));

uint32_t malloc(uint32_t size);
void free(uint32_t ptr);
uint32_t realloc(uint32_t ptr, uint32_t size);
