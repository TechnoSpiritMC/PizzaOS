#pragma once

#include "../include/multiboot.h"

extern uint32_t initial_page_dir[1024];

void initMemory(uint32_t memHigh, uint32_t physicalAllocStart);
void invalidate(uint32_t virtualAddress);
void pmm_init(uint32_t memLow, uint32_t memHigh);
uint32_t pmmAllocPageFrame();
uint32_t* memGetCurrentPageDir();
void memChangePageDir(uint32_t *pd);
void memMapPage(uint32_t virtualAddress, uint32_t physicalAddress, uint32_t flags);
void memUnmapPage(uint32_t virtualAddress);


#define KERNEL_START  0xC0000000
#define KERNEL_MALLOC 0xD0000000

#define REC_PAGE_DIR ((uint32_t*)0xfffff000)
#define REC_PAGE_TABLE(i) ((uint32_t*) (0xffc00000 + ((i) << 12)))

#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_OWNER (1 << 9)