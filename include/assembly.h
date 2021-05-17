#ifndef ASSEMBLY_HEADER
#define ASSEMBLY_HEADER

#include "parser.h"
#include "elf_builder.h"

struct FunctionPos {
    char* func_name;
    int rip;
    long file_p;
};

struct FuncTable {
    FunctionPos* positions;
    int size;
    int capacity;
};

void Assembly(Node* root, const char*  out_filename);
void Encode(Node* root, const char* out_filename);


#endif
