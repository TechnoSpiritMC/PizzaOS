#include "oop.h"

extern "C"{
    #include "malloc.h"
    #include "../include/stdint.h"
}


// 1. Single object allocation
void* operator new(size_t size) {
    return (void*)malloc((uint32_t)size);
}

// 2. Single object deallocation
void operator delete(void* ptr) {
    if (ptr != 0) {
        free((uint32_t)ptr);
    }
}

// 2b. Sized single object deallocation (C++14, required by virtual destructors)
void operator delete(void* ptr, size_t) {
    if (ptr != 0) {
        free((uint32_t)ptr);
    }
}

// 3. Array allocation
void* operator new[](size_t size) {
    return (void*)malloc((uint32_t)size);
}

// 4. Array deallocation
void operator delete[](void* ptr) {
    if (ptr != 0) {
        free((uint32_t)ptr);
    }
}