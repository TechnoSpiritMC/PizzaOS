#pragma once
#include "../include/stdint.h"

void* operator new(size_t size);
void operator delete(void* ptr);

void* operator new[](size_t size);
void operator delete[](void* ptr);