#include "malloc.h"

#include "kmalloc.h"
#include "../include/util.h"

uint32_t malloc(uint32_t size) {

    uint32_t* ptr = (uint32_t*)heapStart;
    struct malloc_block_struct* header;

    //serial_printf("Malloc: Trying to allocate %x bytes. Pointer to heap start is %x\r\n", size, heapStart);

    while (1) {
        header = (struct malloc_block_struct*)ptr;

        //serial_printf("Malloc: Header: %x. Header->used %x, header->size: %x, header->next: %x \r\n", header);

        if (header->used == 0 && header->size >= size) {
            //serial_printf("Found a sector big enough (%x) to store some data (%x) in it. Yayyy\r\n", header->size, size);
            break;
        }

        uint32_t* next = (uint32_t*)(header->next);

        //serial_printf("Malloc: Next header: %x\r\n", next);

        if (next == NULL) {
            //serial_printf("Malloc: No more free sectors, allocating a new one.\r\n");
            uint32_t addToHeap = 0x1000 * CEIL_DIV(size + 12, 0x1000);
            changeHeapSize(heapSize + addToHeap);

            //serial_printf("Malloc: New heap size: %x\r\n", heapSize);

            struct malloc_block_struct* new_block = (struct malloc_block_struct*)((uint8_t*)heapStart + heapSize - addToHeap);
            new_block->size = addToHeap - 12;
            new_block->used = 0;
            new_block->next = NULL;

            //serial_printf("Malloc: New block: %x\r\n", new_block);

            header->next = (struct malloc_block_struct*)((uint8_t*)heapStart + heapSize - addToHeap);
            //serial_printf("Malloc: Header->next: %x\r\n", header->next);
        }

        ptr = (uint32_t*)(header->next);
        //serial_printf("Malloc: new ptr: %x\r\n", ptr);
    }

    uint32_t next = NULL;

    if (header->size - size > sizeThreshold) {
        //serial_printf("The sector is a fat lil boi, let's split it.\r\n", size);

        next = (uint32_t)((uint8_t*)ptr + size + 12);
        struct malloc_block_struct* new_block = (struct malloc_block_struct*)next;
        new_block->size = header->size - size - 12;
        new_block->used = 0;
        new_block->next = header->next;
        header->next = (struct malloc_block_struct*)next;
        header->size = size;

        //serial_printf("Malloc: Header->next: %x\r\n", header->next);
        //serial_printf("Malloc: Header->size: %x\r\n", header->size);
        //serial_printf("Malloc: New block: %x\r\n", new_block);
        //serial_printf("Malloc: Next: %x\r\n", next);
        //serial_printf("Malloc: Header: %x\r\n", header);
        //serial_printf("Malloc: Next header: %x\r\n", header->next);
        //serial_printf("\r\n");
    }

    header->used = 1;
    return (uint32_t)((uint8_t*)ptr + 12);
}

void free(uint32_t ptr) {
    struct malloc_block_struct* header = (struct malloc_block_struct*)((uint8_t*)ptr - 12);
    header->used = 0;

    struct malloc_block_struct* nextHeader = header->next;

    while (nextHeader != NULL && nextHeader->used == 0) {
        header->size += nextHeader->size + 12;
        header->next = nextHeader->next;
        nextHeader = header->next;
    }
}

uint32_t realloc(uint32_t ptr, uint32_t size) {
    // Rule 1: If ptr is NULL, realloc behaves exactly like malloc
    if (ptr == 0) {
        return malloc(size);
    }

    // Rule 2: If size is 0 and ptr is not NULL, realloc behaves like free
    if (size == 0) {
        free(ptr);
        return 0;
    }

    // Get the header of the current block (12 bytes behind the data pointer)
    struct malloc_block_struct* header = (struct malloc_block_struct*)(ptr - 12);
    uint32_t oldSize = header->size;

    // Case 1: The current block is already big enough (Shrinking or perfectly sized)
    if (oldSize >= size) {
        // Optional: You could split the block here if (oldSize - size > sizeThreshold),
        // but keeping it as-is is perfectly safe and fast.
        return ptr;
    }

    // Case 2: Try to grow in-place by checking the next adjacent block
    struct malloc_block_struct* nextBlock = header->next;
    if (nextBlock != NULL && nextBlock->used == 0) {
        // Total combined size if we absorb the next block (including its 12-byte header overhead)
        uint32_t combinedSize = oldSize + 12 + nextBlock->size;

        if (combinedSize >= size) {
            // Absorb the next block
            header->next = nextBlock->next;
            header->size = combinedSize;

            // If there's excess space left over, split it so we don't waste memory
            if (header->size - size > sizeThreshold) {
                uint32_t splitAddr = (uint32_t)((uint8_t*)ptr + size);
                struct malloc_block_struct* splitBlock = (struct malloc_block_struct*)splitAddr;

                splitBlock->size = header->size - size - 12;
                splitBlock->used = 0;
                splitBlock->next = header->next;

                header->next = splitBlock;
                header->size = size;
            }
            return ptr; // Successfully expanded in-place!
        }
    }

    // Case 3: We have to move. Allocate new space, copy data, and free old space.
    uint32_t newPtr = malloc(size);
    if (newPtr == 0) {
        return 0; // Out of memory!
    }

    // Copy the old data up to the old size limit
    memcpy((void*)newPtr, (void*)ptr, oldSize);

    // Free the old chunk
    free(ptr);

    return newPtr;
}