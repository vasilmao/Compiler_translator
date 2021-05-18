#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

struct Variable {
    char* name;
    int usage;
    int real_index;
    bool is_arg;
};

typedef Variable Elem_t;

struct DynamicArray {
    Elem_t* array;
    int  capacity;
    int  size;
    int  arg_cnt;
};

DynamicArray* newDynamicArray();
void DApushBack(DynamicArray* darray, Elem_t element);
int DAfind(DynamicArray* darray, char* element);
void DAdestroy(DynamicArray* darray);
Elem_t DAget(DynamicArray* darray, size_t index);
