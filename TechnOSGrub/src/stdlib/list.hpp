#pragma once

extern "C" {
    #include "../include/stdint.h"

    uint32_t malloc(uint32_t size);
    void free(uint32_t ptr);
    uint32_t realloc(uint32_t ptr, uint32_t size);
}

namespace List {

    struct ArrayList {
        void** items;
        uint32_t capacity;
        uint32_t size;
    };

    // Library API
    void list_init(struct ArrayList* list, uint32_t initial_capacity = 4);
    void list_append(struct ArrayList* list, void* item);
    void* list_get(struct ArrayList* list, uint32_t index);
    void list_free(struct ArrayList* list);
}