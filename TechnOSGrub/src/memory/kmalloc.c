#include "kmalloc.h"

#include "memory.h"
#include "../include/util.h"

static uint32_t heapStart;
static uint32_t heapSize;
static uint32_t threshold;
static bool kmallocInitialized = false;

void kmallocInit(uint32_t initialHeapSize) {
    heapStart = KERNEL_MALLOC;
    heapSize = 0;
    threshold = 0;

    kmallocInitialized = true;

    changeHeapSize(initialHeapSize);
}

void changeHeapSize(int newHeapSize) {
    int oldPageTop = CEIL_DIV(heapSize, 0x1000);
    int newPageTop = CEIL_DIV(newHeapSize, 0x1000);

    int diff = newPageTop - oldPageTop;

    for (int i = 0; i < diff; i++) {
        uint32_t phys = pmmAllocPageFrame();
        memMapPage(KERNEL_MALLOC + oldPageTop * 0x1000 + i * 0x1000, phys, PAGE_FLAG_WRITE);
    }
}