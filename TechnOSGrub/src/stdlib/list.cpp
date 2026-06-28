#include "list.hpp"

namespace List {

    void list_init(ArrayList* list, uint32_t initial_capacity) {
        list->capacity = initial_capacity;
        list->size = 0;

        uint32_t bytes_to_allocate = list->capacity * sizeof(void*);
        list->items = (void**)malloc(bytes_to_allocate);
    }

    void list_append(ArrayList* list, void* item) {
        if (list->size >= list->capacity) {
            list->capacity *= 2;
            uint32_t new_byte_size = list->capacity * sizeof(void*);

            uint32_t reallocated_ptr = realloc((uint32_t)list->items, new_byte_size);
            list->items = (void**)reallocated_ptr;
        }

        list->items[list->size] = item;
        list->size++;
    }

    void* list_get(ArrayList* list, uint32_t index) {
        if (index >= list->size) {
            return nullptr; 
        }
        return list->items[index];
    }

    void list_free(ArrayList* list) {
        if (list->items) {
            free((uint32_t)list->items);
            list->items = nullptr;
        }
        list->size = 0;
        list->capacity = 0;
    }
}