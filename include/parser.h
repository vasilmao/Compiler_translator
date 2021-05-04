#ifndef PARSE_HEADER
#define PARSE_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "DynamicArray.h"
#include "file_reader.h"


union Value {
    unsigned char math; // MathOP
    int64_t num;   //double value
    char* name;   //function or variable
    DynamicArray* variables; // for functions (d_type has it)
    int     var_number; // for ASssg_type, variable number in array
};

struct Node {
    Node* left;
    Node* right;
    char type;
    Value value;
};

enum NodeType
{                 //                                     In sir gay's tree
    D_TYPE,       // new func                         |   fict
    DECL_TYPE,    // variable declaration             |   =
    ID_TYPE,      // variable or function             |   name of func or var
    ARG_TYPE,     // bunch of arguments               |   args

    COMP_TYPE,    // :                                |   {
    STAT_TYPE,    // ';' or loop/condition            |   S

    COND_TYPE,    // condition                        |   if
    IFEL_TYPE,    // if-else                          |   if-else
    LOOP_TYPE,    // while                            |   while
    ASSG_TYPE,    // '='                              |   =

    CALL_TYPE,    // call func                        |   call
    JUMP_TYPE,    // return                           |   return

    MATH_TYPE,    // + - * / < > <= >= == !=          |
    NUMB_TYPE,    // double                           |   *value of number*
    BLANK_LINE_TYPE,

    TYPES_COUNT   // We will add scan, print, round   |   p.s. Fictive nodes's values == 0
};

enum MathOp
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_EQUAL,
    OP_NE,
    OP_LOE,
    OP_GOE,
    OP_LESS,
    OP_GREATER
};



struct Text {
    char* s;
    size_t p;
};

Node* parseString(char* s);

void graphDump(Node* root);

#endif
