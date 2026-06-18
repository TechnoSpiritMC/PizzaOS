//
// Created by denis on 6/14/2026.
//

#include "memory.h"
#include "../include/stdint.h"
#include "../stdlib/stdio.h"
#include "../include/util.h"

static uint32_t pageFrameMin;
static uint32_t pageFrameMax;
static uint32_t totalAllocated;

#define NUM_PAGE_DIRS 256
#define NUM_PAGE_FRAMES (0x10000000 / 0x1000)

uint8_t physicalMemoryBitmap[NUM_PAGE_FRAMES / 8]; // Dynamically, bit array..?

static uint32_t pageDirs[NUM_PAGE_DIRS][1024] __attribute__((aligned(4096)));
static uint8_t pageDirUsed[NUM_PAGE_DIRS];

int mem_num_vpages;

void pmm_init(uint32_t memLow, uint32_t memHigh) {
    pageFrameMin = CEIL_DIV(memLow, 0x1000);
    pageFrameMax = CEIL_DIV(memHigh, 0x1000);

    totalAllocated = 0;

    memset(physicalMemoryBitmap, 0, sizeof(physicalMemoryBitmap));
}

uint32_t* memGetCurrentPageDir() {
    uint32_t pd;
    asm volatile("mov %%cr3, %0" : "=r"(pd));
    pd += KERNEL_START;

    return (uint32_t*) pd;
}

void memChangePageDir(uint32_t *pd) {
    pd = (uint32_t*)(((uint32_t) pd) - KERNEL_START);
    asm volatile("mov %0, %%eax \n mov %%eax, %%cr3\n" :: "m"(pd));
}

void syncPageDirs() {
    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (pageDirUsed[i]) {
            uint32_t *pd = pageDirs[i];

            for (int j = 768; j < 1023; j++) {
                pd[j] = initial_page_dir[j] & ~PAGE_FLAG_OWNER;
            }
        }
    }
}

void memMapPage(uint32_t virtualAddress, uint32_t physicalAddress, uint32_t flags) {
    uint32_t *prevPageDir = 0;

    /* --- COMMENTED OUT TO PREVENT CR3 SHIFTING ---
    if (virtualAddress >= KERNEL_START) {
        prevPageDir = memGetCurrentPageDir();

        if (prevPageDir != initial_page_dir) {
            memChangePageDir(initial_page_dir);
        }
    }
    */

    uint32_t pdIndex = virtualAddress >> 22;
    uint32_t ptIndex = (virtualAddress >> 12) & 0x3FF;

    uint32_t *pageDir = REC_PAGE_DIR;
    uint32_t* pt = REC_PAGE_TABLE(pdIndex);

    if (!(pageDir[pdIndex] & PAGE_FLAG_PRESENT)) {
        uint32_t ptPAddr = pmmAllocPageFrame();
        pageDir[pdIndex] = ptPAddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;

        uint32_t currentCr3;
        asm volatile("mov %%cr3, %0" : "=r"(currentCr3));
        asm volatile("mov %0, %%cr3" :: "r"(currentCr3));

        for (uint32_t i = 0; i < 1024; i++) {
            pt[i] = 0;
        }

        invalidate((uint32_t) pt);
        invalidate(virtualAddress);

        for (uint32_t i = 0; i < 1024; i++) {
            pt[i] = 0;
        }
    }

    pt[ptIndex] = physicalAddress | PAGE_FLAG_PRESENT | flags;
    mem_num_vpages++;
    invalidate(virtualAddress);

    /* --- COMMENTED OUT TO PREVENT CR3 SHIFTING ---
    if (prevPageDir != 0) {
        syncPageDirs();
        if (prevPageDir != initial_page_dir) {
            memChangePageDir(prevPageDir);
        }
    }
    */
}

void initMemory(uint32_t memHigh, uint32_t physicalAllocStart) {
    mem_num_vpages = 0;
    initial_page_dir[0] = 0;
    invalidate(0);
    initial_page_dir[1023] = ((uint32_t) initial_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate(0xfffff000);

    pmm_init(physicalAllocStart, memHigh);
    memset(pageDirs, 0, 4096 * NUM_PAGE_DIRS);
    memset(pageDirUsed, 0, NUM_PAGE_DIRS);
}

void invalidate(uint32_t virtualAddress) {
    asm volatile("invlpg %0"::"m"(virtualAddress));
}

uint32_t pmmAllocPageFrame() {
    uint32_t start = pageFrameMin / 8 + ((pageFrameMin & 7) != 0? 1: 0);
    uint32_t end = pageFrameMax / 8 - ((pageFrameMax & 7) != 0? 1: 0);

    for (uint32_t b = start; b <= end; b++) {
        uint8_t byte = physicalMemoryBitmap[b];
        if (byte == 0xFF) continue;

        for (uint32_t i = 0; i < 8; i++) {
            bool used = byte >> i & 1;

            if (!used) {
                byte ^= (-1 ^byte) & (1 << i);
                physicalMemoryBitmap[b] = byte;
                totalAllocated++;

                uint32_t addr = (b*8+i) * 0x1000;
                return addr;
            }
        }
    }

    return 0;
}