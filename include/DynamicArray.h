#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

typedef char* Elem_t;

struct DynamicArray {
    Elem_t* array;
    int  capacity;
    int  size;
};

DynamicArray* newDynamicArray();
void DApushBack(DynamicArray* darray, Elem_t element);
int DAfind(DynamicArray* darray, Elem_t element);
void DAdestroy(DynamicArray* darray);
Elem_t DAget(DynamicArray* darray, size_t index);
