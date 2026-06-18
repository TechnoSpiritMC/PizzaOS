#include "kmalloc.h"

#include "memory.h"
#include "../include/util.h"
#include "malloc.h"

uint32_t heapStart;
uint32_t heapSize;
uint32_t threshold;
bool kmallocInitialized = false;

void kmallocInit(uint32_t initialHeapSize) {
    heapStart = KERNEL_MALLOC;
    heapSize = 0;
    threshold = 0;

    kmallocInitialized = true;

    changeHeapSize(initialHeapSize);
    memset((void*)heapStart, 0, heapSize);

    struct malloc_block_struct* start = (struct malloc_block_struct*)(heapStart);
    start->size = initialHeapSize;
    start->used = 0;
    start->next = NULL;
}

void changeHeapSize(int newHeapSize) {
    int oldPageTop = CEIL_DIV(heapSize, 0x1000);
    int newPageTop = CEIL_DIV(newHeapSize, 0x1000);

    int diff = newPageTop - oldPageTop;

    for (int i = 0; i < diff; i++) {
        uint32_t phys = pmmAllocPageFrame();
        memMapPage(KERNEL_MALLOC + oldPageTop * 0x1000 + i * 0x1000, phys, PAGE_FLAG_WRITE);
    }

    heapSize = newHeapSize;
}