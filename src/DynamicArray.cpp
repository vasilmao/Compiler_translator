#include "DynamicArray.h"

DynamicArray* newDynamicArray() {
    DynamicArray* array = (DynamicArray*)calloc(1, sizeof(DynamicArray));
    //printf("array calloced as %p\n", array);
    array->array = (Elem_t*)calloc(50, sizeof(Elem_t));
    //printf("array->array calloced as %p\n", array->array);
    array->capacity = 50;
    array->size = 0;
    return array;
}

void DAresize(DynamicArray* darray) {
    darray->capacity *= 2;
    darray->array = (Elem_t*)realloc(darray->array, darray->capacity * sizeof(Elem_t));
}

void DApushBack(DynamicArray* darray, Elem_t element) {
    if (darray->size == darray->capacity) {
        DAresize(darray);
    }
    darray->array[darray->size++] = element;
    darray->array[darray->size - 1].real_index = darray->size - 1;
}

Elem_t DAget(DynamicArray* darray, size_t index) {
    assert(index < darray->size);
    return darray->array[index];
}

int DAfind(DynamicArray* darray, char* element) {
    for (int i = 0; i < darray->size; ++i) {
        printf("i - %d\n", i);
        printf("%p %p\n", element, darray->array[i].name);
        if (strcmp(element, darray->array[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void DAdestroy(DynamicArray* darray) {
    free(darray->array);
    free(darray);
}
