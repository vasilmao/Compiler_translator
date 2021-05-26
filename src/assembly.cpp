#include "assembly.h"

typedef byte byte;

const int optimized_vars = 4;
const int aligning = 0x1000;


#define AddCodeConst(a, b) (AddCode(a, b, sizeof(b)))

// so i can't write wrong string size

struct CodeBuffer {
    byte* buffer;
    byte* last_command;
    int size;
    int capacity;
};

struct BytecodeData {
    DynamicArray* variables;
    CodeBuffer* buffer;
    FuncTable* func_table;
    FuncTable* func_needed;
    int optimize;
    // int rip;
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

CodeBuffer* CreateCodeBuffer();
void AddCode(CodeBuffer* buf, const byte* code, int len);
void DestroyCodeBuffer(CodeBuffer* buf);

FuncTable* CreateFuncTable();
void AddFunctionPos(FuncTable* table, char* func_name, uint64_t position);
void DestroyFuncTable(FuncTable* table);

void IncludeStdlib(FILE* out_file);
void IncludeStdlibBin(CodeBuffer* buf, int in_buf, int out_buf, int new_line);

int GetArgCnt(Node* arg_node);
void ASMFdecl(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMName(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMComp(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMCall(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMMath(Node* node, DynamicArray* vars, FILE* out_file, int optimize);
void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file, int optimize);

void BytecodeFdecl(Node* node, BytecodeData* data);
void BytecodeVarDecl(Node* node, BytecodeData* data);
void BytecodeName(Node* node, BytecodeData* data);
void BytecodeArgument(Node* node, BytecodeData* data);
void BytecodeComp(Node* node, BytecodeData* data);
void BytecodeStatement(Node* node, BytecodeData* data);
void BytecodeCondition(Node* node, BytecodeData* data);
void BytecodeIfelse(Node* node, BytecodeData* data);
void BytecodeLoop(Node* node, BytecodeData* data);
void BytecodeAssg(Node* node, BytecodeData* data);
void BytecodeCall(Node* node, BytecodeData* data);
void BytecodeReturn(Node* node, BytecodeData* data);
void BytecodeMath(Node* node, BytecodeData* data);
void BytecodeConstant(Node* node, BytecodeData* data);



CodeBuffer* CreateCodeBuffer() {
    CodeBuffer* buf = (CodeBuffer*)calloc(1, sizeof(CodeBuffer));
    buf->capacity = aligning;
    buf->buffer = (byte*)calloc(buf->capacity, sizeof(byte));
    return buf;
}

void AddCode(CodeBuffer* buf, const byte* code, int len) {
    if (len + buf->size > buf->capacity) {
        buf->capacity += aligning;
        buf->buffer = (byte*)realloc(buf->buffer, buf->capacity);
    }
    buf->last_command = buf->buffer + buf->size;
    memcpy(buf->last_command, code, len);
    buf->size += len;
}

void DestroyCodeBuffer(CodeBuffer* buf) {
    free(buf->buffer);
    free(buf);
}


void ASMParseNode(Node* node, DynamicArray* vars, FILE* out_file, int optimize) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case D_TYPE:    ASMFdecl    (node, vars, out_file, optimize);   break;
        case DECL_TYPE: ASMVarDecl  (node, vars, out_file, optimize);   break;
        case ID_TYPE:   ASMName     (node, vars, out_file, optimize);   break;
        case ARG_TYPE:  ASMArgument (node, vars, out_file, optimize);   break;
        case COMP_TYPE: ASMComp     (node, vars, out_file, optimize);   break;
        case STAT_TYPE: ASMStatement(node, vars, out_file, optimize);   break;
        case COND_TYPE: ASMCondition(node, vars, out_file, optimize);   break;
        case IFEL_TYPE: ASMIfelse   (node, vars, out_file, optimize);   break;
        case LOOP_TYPE: ASMLoop     (node, vars, out_file, optimize);   break;
        case ASSG_TYPE: ASMAssg     (node, vars, out_file, optimize);   break;
        case CALL_TYPE: ASMCall     (node, vars, out_file, optimize);   break;
        case JUMP_TYPE: ASMReturn   (node, vars, out_file, optimize);   break;
        case MATH_TYPE: ASMMath     (node, vars, out_file, optimize);   break;
        case NUMB_TYPE: ASMConstant (node, vars, out_file, optimize);   break;
    }
}

void BytecodeParseNode(Node* node, BytecodeData* data) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case D_TYPE:    BytecodeFdecl    (node, data);   break;
        case DECL_TYPE: BytecodeVarDecl  (node, data);   break;
        case ID_TYPE:   BytecodeName     (node, data);   break;
        case ARG_TYPE:  BytecodeArgument (node, data);   break;
        case COMP_TYPE: BytecodeComp     (node, data);   break;
        case STAT_TYPE: BytecodeStatement(node, data);   break;
        case COND_TYPE: BytecodeCondition(node, data);   break;
        case IFEL_TYPE: BytecodeIfelse   (node, data);   break;
        case LOOP_TYPE: BytecodeLoop     (node, data);   break;
        case ASSG_TYPE: BytecodeAssg     (node, data);   break;
        case CALL_TYPE: BytecodeCall     (node, data);   break;
        case JUMP_TYPE: BytecodeReturn   (node, data);   break;
        case MATH_TYPE: BytecodeMath     (node, data);   break;
        case NUMB_TYPE: BytecodeConstant (node, data);   break;
    }
}

FuncTable* CreateFuncTable() {
    FuncTable* table = (FuncTable*) calloc(1, sizeof(FuncTable*));
    table->size = 0;
    table->capacity = 100;
    table->positions = (FunctionPos*)calloc(table->capacity, sizeof(FunctionPos));
    return table;
}

void AddFunctionPos(FuncTable* table, char* func_name, int rip, long file_p) {
    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->positions = (FunctionPos*)realloc(table->positions, table->capacity * sizeof(FunctionPos));
    }
    int func_name_len = strlen(func_name);
    table->positions[table->size].func_name = (char*)calloc(func_name_len + 1, sizeof(char));
    strncpy(table->positions[table->size].func_name, func_name, func_name_len);
    table->positions[table->size].rip = rip;
    table->positions[table->size].file_p = file_p;
    table->size++;
}

void DestroyFuncTable(FuncTable* table) {
    for (int i = 0; i < table->size; ++i) {
        free(table->positions[i].func_name);
    }
    free(table->positions);
    free(table);
}

const byte init_rbp[3] = {0x48, 0x89, 0xE5};                                      // mov rbp, rsp \n

const byte create_place_for_vars[7] = {0x48, 0x81, 0xEC, 0x00, 0x00, 0x00, 0x00}; // sub rsp, 0 (to be patched)

const byte clear_vars[7] = {0x48, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x00};            // add rsp, 0 (to be patched)

const byte end_proggram[10] = {0xB8, 0x3C, 0x00, 0x00, 0x00,                      // mov rax, 0x3C
                               0x48, 0x31, 0xFF,                                  // xor rdi, rdi
                               0x0F, 0x05};                                       // syscall


void Encode(Node* root, const char* out_filename, int optimize) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");

    ElfHeader* elf_header = CreateElfHeader();

    ProgramHeader* elf_prog_header = CreateProgramHeader();
    SetHeadersHeader(elf_prog_header);

    ProgramHeader* data_header = CreateProgramHeader();
    SetDataHeader(data_header);

    ProgramHeader* code_header = CreateProgramHeader();
    SetCodeHeader(code_header);

    CodeBuffer* buf = CreateCodeBuffer();

    int stdlib_start       = *((int*)code_header->virtual_address);
    int data_segment_start = *((int*)data_header->virtual_address);
    int  in_buffer_pos     = data_segment_start;
    int out_buffer_pos     = data_segment_start + 1;
    int   new_line_pos     = data_segment_start + 20;

    IncludeStdlibBin(buf, in_buffer_pos, out_buffer_pos, new_line_pos);

    int entry_point = stdlib_start + buf->size;
    WriteLittleInd64(entry_point, elf_header->values + 24);

    DynamicArray* current_vars = root->value.variables;
    FuncTable* func_table = CreateFuncTable();
    FuncTable* func_needed = CreateFuncTable();
    // add stdlib functions to func table
   // printf("input start: %x\n", stdlib_start);
    AddFunctionPos(func_table, "input", 0, 0);
    AddFunctionPos(func_table, "print", 0x81, 0x81);

    BytecodeData data = {current_vars, buf, func_table, func_needed, optimize};

    AddCodeConst(buf, create_place_for_vars);
    int var_cnt = data.variables->size * 8;
    WriteLittleInd32(var_cnt, buf->buffer + buf->size - sizeof(int));
    AddCodeConst(buf, init_rbp);

    BytecodeParseNode(root, &data);

    AddCodeConst(buf, clear_vars);
    WriteLittleInd32(var_cnt, buf->buffer + buf->size - sizeof(int));
    AddCodeConst(buf, end_proggram);

    WriteLittleInd64(buf->size, code_header->file_size);
    WriteLittleInd64(buf->size, code_header->mem_size);

    for (int i = 0; i < func_needed->size; ++i) {
        for (int j = 0; j < func_table->size; ++j) {
            if (strcmp(func_needed->positions[i].func_name, func_table->positions[j].func_name) == 0) {
                int distance = func_table->positions[j].rip - func_needed->positions[i].rip;
                WriteLittleInd32(distance, buf->buffer + func_needed->positions[i].file_p);
                break;
            }
        }
    }

    byte lot_of_zeroes[aligning];
    memset(lot_of_zeroes, 0, aligning);

    fwrite(elf_header, sizeof(char), sizeof(ElfHeader), out_file);
    fwrite(elf_prog_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(data_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(code_header, sizeof(char), sizeof(ProgramHeader), out_file);

    fwrite(lot_of_zeroes, sizeof(byte), aligning - ftell(out_file), out_file); // align
    lot_of_zeroes[0x14] = 0x0a; // new_line
    fwrite(lot_of_zeroes, sizeof(byte), aligning, out_file); // aligned data segment
    lot_of_zeroes[0x14] = 0x00; // no new_line
    fwrite(buf->buffer, sizeof(byte), buf->capacity, out_file);

    fclose(out_file);
}

void Assembly(Node* root, const char* out_filename, int optimize) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");
    DynamicArray* current_vars = root->value.variables;
   // printf("yeah %p\n", current_vars);
    IncludeStdlib(out_file);
    fprintf(out_file, "section .text\nglobal _start\n");
    fprintf(out_file, "_start:\n");
    fprintf(out_file, "sub rsp, %d\n ; 48_81 (83 if imm8 instead of imm32)_\n", current_vars->size * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    ASMParseNode(root, current_vars, out_file, optimize);
    fprintf(out_file, "add rsp, %d\nmov rax, 0x3C\nxor rdi, rdi\nsyscall\n", current_vars->size * 8);
   // printf("bruh1\n");
    fclose(out_file);
   // printf("bruh2\n");
}

void IncludeStdlibBin(CodeBuffer* buf, int in_buf, int out_buf, int new_line) {
    FILE* bin_stdlib = NULL;
    open_file(&bin_stdlib, "stdlib.bin", "r");
    char* char_buffer = NULL;
    uint64_t buffer_size = 0;
    read_buffer(&char_buffer, &buffer_size, "stdlib.bin", bin_stdlib);
    // file closed

    AddCode(buf, (byte*)(char_buffer), (int)buffer_size);
    // in_buf += 0x400000;
    // out_buf += 0x400000;
    // new_line += 0x400000;
   // printf("kek!!! %x\n", in_buf);
    WriteLittleInd64(in_buf,   buf->buffer + 0x10); //yes, magic constants of where are stdlib string pointers
    WriteLittleInd32(in_buf,   buf->buffer + 0x22);
    WriteLittleInd64(in_buf,   buf->buffer + 0x3B);
    WriteLittleInd32(in_buf,   buf->buffer + 0x4E);
    WriteLittleInd32(out_buf,  buf->buffer + 0x99);
    WriteLittleInd64(out_buf,  buf->buffer + 0xA0);
    WriteLittleInd32(out_buf,  buf->buffer + 0xD9);
    WriteLittleInd64(out_buf,  buf->buffer + 0xF5);
    WriteLittleInd64(new_line, buf->buffer + 0x10E);


    free(char_buffer);
}

void IncludeStdlib(FILE* out_file) {
    FILE* stdlib_file = NULL;
    open_file(&stdlib_file, "stdlib.asm", "r");
    char* buffer = NULL;
    uint64_t buffer_size = 0;
    read_buffer(&buffer, &buffer_size, "stdlib.asm", stdlib_file);
    fwrite(buffer, sizeof(char), buffer_size, out_file);
    free(buffer);
}


void ASMFdecl(Node* node, DynamicArray* vars, FILE* out_file, int optimize) {
    assert(node);
    Node* fname = node->right;
    int arg_cnt = node->value.variables->arg_cnt;
    int var_cnt = node->value.variables->size;
    fprintf(out_file, "jmp %s_END\n", fname->value.name);
    fprintf(out_file, "%s: ;eeee fdecl\n", fname->value.name);
    fprintf(out_file, "sub rsp, %d\n", (var_cnt - arg_cnt) * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    vars = node->value.variables;
    if (optimize & 1) {
        for (int i = 0; i < MIN(vars->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = vars->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (vars->array[var_index_sorted].is_arg) {
                var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - vars->arg_cnt;
            }
            // int var_index_sorted = DAfind(vars, node->left->value.name);
            if (optimize & 1) {
                if (var_index_sorted < optimized_vars) {
                    fprintf(out_file, "mov r%d, [rbp + %d]\n", i + 8, var_offset * 8);
                    // return;
                }
            }
        }
    }
   // printf("yeah fdecl %p\n", fname);
    // arguments in stack, rbp + x
    assert(node->value.variables);
    ASMParseNode(node->right->left, node->value.variables, out_file, optimize);
    fprintf(out_file, "add rsp, %d ; this was fdecl\n", (var_cnt - arg_cnt) * 8);
    fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
}

// just move between [rbp + 0] and rax
const byte mov_r64new_mem[7] = {0x4C, 0x8B, 0x85, 0x00, 0x00, 0x00, 0x00};
const byte mov_mem_r64new[7] = {0x4C, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00};
const byte mov_r64new_rax[3] = {0x49, 0x89, 0xC0};
const byte mov_rax_r64new[3] = {0x4C, 0x89, 0xC0};

void OptimRestoreReg(CodeBuffer* buffer, int reg_num, int mem_ofs) {
   // printf("yes restore\n");
    assert(0 <= reg_num && reg_num <= 8);
    int reg_code = 0x85 + reg_num * 8;
    byte final_code[7];
    for (int i = 0; i < 7; ++i) {
        final_code[i] = mov_r64new_mem[i];
    }
    final_code[2] = (byte)reg_code;
    WriteLittleInd32(mem_ofs, final_code + 3);
   // printf("%x, %x\n", (int)final_code[1], (int)final_code[2]);
    AddCode(buffer, final_code, 7);
   // printf("%x\n", buffer->size);
}

void OptimSaveReg(CodeBuffer* buffer, int reg_num, int mem_ofs) {
   // printf("yes save\n");
    assert(0 <= reg_num && reg_num <= 8);
    int reg_code = 0x85 + reg_num * 8;
   // printf("%d\n", reg_num);
    byte final_code[7];
    for (int i = 0; i < 7; ++i) {
        final_code[i] = mov_mem_r64new[i];
    }
    final_code[2] = (byte)reg_code;
    WriteLittleInd32(mem_ofs, final_code + 3);
   // printf("%x, %x\n", (int)final_code[1], (int)final_code[2]);
    AddCode(buffer, final_code, 7);
}

void OptimSetReg(CodeBuffer* buffer, int reg_num) {
   // printf("reg set\n");
   // printf("%x\n", buffer->size);
    assert(0 <= reg_num && reg_num <= 8);
    int reg_code = 0xC0 + reg_num;
    byte final_code[3];
    for (int i = 0; i < 3; ++i) {
        final_code[i] = mov_r64new_rax[i];
    }
    final_code[2] = (byte)reg_code;
    AddCode(buffer, final_code, 3);
   // printf("%x\n", buffer->size);
}

void OptimGetReg(CodeBuffer* buffer, int reg_num) {
   // printf("reg geet\n");
   // printf("%x\n", buffer->size);
    assert(0 <= reg_num && reg_num <= 8);
    int reg_code = 0xC0 + reg_num * 8;
    byte final_code[3];
    for (int i = 0; i < 3; ++i) {
        final_code[i] = mov_rax_r64new[i];
    }
    final_code[2] = (byte)reg_code;
    AddCode(buffer, final_code, 3);
}

const byte jmp_rel[5]  = {0xE9, 0x00, 0x00, 0x00, 0x00}; // jmp (place)
const byte ret[1] = {0xC3};                              // ret


void BytecodeFdecl(Node* node, BytecodeData* data) {
    assert(node);
    Node* fname = node->right;

    CodeBuffer* buf = data->buffer;

    AddCodeConst(buf, jmp_rel);
    byte* jmp_buf_pointer = buf->last_command + 1;
    int jmp_rip = buf->size;

    AddFunctionPos(data->func_table, fname->value.name, buf->size, 0);

    DynamicArray* vars = node->value.variables;

    AddCodeConst(buf, create_place_for_vars);
    DynamicArray* current_variables = data->variables;
    data->variables = node->value.variables;
    assert(node->value.variables);

    // byte var_cnt_str[4];
    int var_cnt = (data->variables->size - data->variables->arg_cnt) * 8;
    // WriteLittleInd32(var_cnt * 8, var_cnt_str);
    // fwrite(var_cnt_str, sizeof(char), 4, out_file);
    // printf("yeah func decl!\n");
    WriteLittleInd32(var_cnt, buf->buffer + buf->size - sizeof(int));
    AddCodeConst(buf, init_rbp);

    if (data->optimize & 1) {
        for (int i = 0; i < MIN(vars->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = vars->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (vars->array[var_index_sorted].is_arg) {
                var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - vars->arg_cnt;
            }
            // int var_index_sorted = DAfind(vars, node->left->value.name);
            if (data->optimize & 1) {
                if (var_index_sorted < optimized_vars) {
                    OptimRestoreReg(data->buffer, i, var_offset * 8);
                    // fprintf(out_file, "mov r%d, [rbp + %d]\n", i, var_offset * 8);
                    // return;
                }
            }
        }
    }

   // printf("yeah fdecl %p\n", fname);

    // arguments in stack, rbp + x
    BytecodeParseNode(node->right->left, data);
    // fwrite(clear_vars, sizeof(char), 3, out_file);
    AddCodeConst(buf, clear_vars);
    // fwrite(var_cnt_str, sizeof(char), 4, out_file);
    WriteLittleInd32(var_cnt, buf->buffer + buf->size - sizeof(int));
    data->variables = current_variables;

    // --------fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
    // fwrite(ret, sizeof(char), 1, out_file);
    AddCodeConst(buf, ret);

    // write jump
    int jmp_relative_distance = buf->size - jmp_rip;
   // printf("func jump distance: %d\n", jmp_relative_distance);
    // long current_file_p = ftell(out_file);
    WriteLittleInd32(jmp_relative_distance, jmp_buf_pointer);

    // byte number[4];
    // WriteLittleInd32(jmp_relative_distance, number);
    // fseek(out_file, jmp_file_pointer, 0);
    // fwrite(number, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);

}

void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file, int optimize) {
    assert(node);
    ASMParseNode(node->right, vars, out_file, optimize);
    // var (left_child) = expression (right_child), result of expression is needed in rax
   // printf("kk\n\n");
    int var_index_sorted = DAfind(vars, node->left->value.name);
    if (optimize & 1) {
        if (var_index_sorted < optimized_vars) {
            fprintf(out_file, "mov r%d, rax\n", var_index_sorted + 8);
            return;
        }
    }
    int var_index_unsorted = vars->array[var_index_sorted].real_index;
    int var_offset = 0;
    if (vars->array[var_index_sorted].is_arg) {
        var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
    } else {
        var_offset = var_index_unsorted - vars->arg_cnt;
    }
   // printf("yeah %d * 8\n", var_offset);
    var_offset *= 8;
    fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", var_offset);
}

const byte var_decl_short[4] = {0x48, 0x89, 0x45, 0x00};                     // mov [rbp + x], rax where -128 <= x <= 127
const byte var_decl_long[7] = {0x48, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00};    // mov [rbp + x], rax where x is any int

void BytecodeVarDecl(Node* node, BytecodeData* data) {
    assert(node);
    BytecodeParseNode(node->right, data);
    CodeBuffer* buf = data->buffer;
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", node->value.var_number * 8);
    int var_index_sorted = DAfind(data->variables, node->left->value.name);
    if (data->optimize & 1) {
       // printf("--------------------------------------------working\n");
        if (var_index_sorted < optimized_vars) {
            OptimSetReg(data->buffer, var_index_sorted);
            // fprintf(out_file, "mov r%d, rax\n", var_index_sorted + 8);
            return;
        }
    }
    int var_index_unsorted = data->variables->array[var_index_sorted].real_index;
    int var_offset = 0;
    if (data->variables->array[var_index_sorted].is_arg) {
        var_offset = (data->variables->size - data->variables->arg_cnt + 2 + var_index_unsorted);
    } else {
        var_offset = var_index_unsorted - data->variables->arg_cnt;
    }
   // printf("yeah %d * 8\n", var_offset);
    var_offset *= 8;
    AddCodeConst(buf, var_decl_long);
    WriteLittleInd32(var_offset, buf->buffer + buf->size - sizeof(int));

}

void ASMName(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
    // i need to find var number and mov it to rax
    int var_index_sorted = DAfind(vars, node->value.name);
    if (optimize & 1) {
        if (var_index_sorted < optimized_vars) {
            fprintf(out_file, "mov rax, r%d\n", var_index_sorted + 8);
            return;
        }
    }
    int var_index_unsorted = vars->array[var_index_sorted].real_index;
    int var_offset = 0;
    if (vars->array[var_index_sorted].is_arg) {
        var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
    } else {
        var_offset = var_index_unsorted - vars->arg_cnt;
    }
   // printf("name %d\n", var_offset * 8);
    fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", var_offset * 8);

}

const byte var_get_short[4] = {0x48, 0x8B, 0x45, 0x00};                   // mov rax, [rbp + x] where -128 <= x <= 127
const byte var_get_long[7]  = {0x48, 0x8B, 0x85, 0x00, 0x00, 0x00, 0x00}; // mov rax, [rbp + x] where x is any int

void BytecodeName(Node* node, BytecodeData* data) {
    assert(node);
    CodeBuffer* buf = data->buffer;
   // printf("name!!!!! %p\n", node);
    // i need to find var number and mov it to rax
    int var_index_sorted = DAfind(data->variables, node->value.name);
    if (data->optimize & 1) {
        if (var_index_sorted < optimized_vars) {
            OptimGetReg(data->buffer, var_index_sorted);
            //fprintf(out_file, "mov rax, r%d\n", var_index_sorted + 8);
            return;
        }
    }
    int var_index_unsorted = data->variables->array[var_index_sorted].real_index;
    int var_offset = 0;
    if (data->variables->array[var_index_sorted].is_arg) {
        var_offset = (data->variables->size - data->variables->arg_cnt + 2 + var_index_unsorted);
    } else {
        var_offset = var_index_unsorted - data->variables->arg_cnt;
    }
   // printf("yeah %d * 8\n", var_offset);
    var_offset *= 8;
    // fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", DAfind(vars, node->value.name) * 8);
    AddCodeConst(buf, var_get_long);
    WriteLittleInd32(var_offset, buf->buffer + buf->size - sizeof(int));
}

void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    // need to push arguments, but first argument must be pushed last, so parse right son
    if (node->right != NULL) {
        ASMParseNode(node->right, vars, out_file, optimize);
    }
    ASMParseNode(node->left, vars, out_file, optimize);
    fprintf(out_file, "push rax ; this was argument passing\n");
    assert(node);
}

const byte add_argument[1] = {0x50}; //push rax

void BytecodeArgument(Node* node, BytecodeData* data){
    // need to push arguments, but first argument must be pushed last, so parse right son
    assert(node);
    CodeBuffer* buf = data->buffer;
    if (node->right != NULL) {
        BytecodeParseNode(node->right, data);
    }
    BytecodeParseNode(node->left, data);
    // fprintf(out_file, "push rax ; this was argument passing\n");
    // fwrite(add_argument, sizeof(char), 1, out_file);
    AddCodeConst(buf, add_argument);
}

void ASMComp(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
   // printf("yeah compound\n");
    ASMParseNode(node->right, vars, out_file, optimize);
}

void BytecodeComp(Node* node, BytecodeData* data){
    assert(node);
   // printf("yeah compound\n");
    BytecodeParseNode(node->right, data);
}

void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
   // printf("yeah statement %p %p\n", node->left, node->right);
    ASMParseNode(node->left, vars, out_file, optimize);
    ASMParseNode(node->right, vars, out_file, optimize);
}

void BytecodeStatement(Node* node, BytecodeData* data){
    assert(node);
   // printf("yeah statement %p %p\n", node->left, node->right);
    BytecodeParseNode(node->left, data);
    BytecodeParseNode(node->right, data);
}

void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
    ASMParseNode(node->left, vars, out_file, optimize);
    fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    ASMParseNode(node->right->left, vars, out_file, optimize);
    fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    ASMParseNode(node->right->right, vars, out_file, optimize);
    fprintf(out_file, "LEND%p:\nnop\nnop ; condition end\n", node);
}

const byte check_if_cond_true[4] = {0x48, 0x83, 0xF8, 0x00};          // cmp rax, 0
const byte jump_to_else[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};    // je (place)

void BytecodeCondition(Node* node, BytecodeData* data){
    assert(node);
    BytecodeParseNode(node->left, data);
    CodeBuffer* buf = data->buffer;
    // fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    // fwrite(check_if_cond_true, sizeof(char), 4, out_file);
    AddCodeConst(buf, check_if_cond_true);
    // fwrite(jump_to_else, sizeof(char), 6, out_file);
    AddCodeConst(buf, jump_to_else);
    int insert_else_rip = buf->size;
    byte* insert_else_buf_p = buf->buffer + buf->size - sizeof(int);
    BytecodeParseNode(node->right->left, data);

    // fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    // fwrite(jmp_rel, sizeof(char), 5, out_file);
    AddCodeConst(buf, jmp_rel);
    int insert_end_rip = buf->size;
    byte* insert_end_buf_p = buf->buffer + buf->size - sizeof(int);
    // now insert data->rip as else
    // long current_file_p = ftell(out_file);
    // long insert_end_file_p = current_file_p - 4;
    // int insert_end_rip = data->rip;
    // fseek(out_file, insert_else_file_p, 0);
    // byte rip_str[4];
    // WriteLittleInd32(data->rip - insert_else_rip, rip_str);
    // fwrite(rip_str, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);
    int jmp_diff = buf->size - insert_else_rip;
    WriteLittleInd32(jmp_diff, insert_else_buf_p);
    BytecodeParseNode(node->right->right, data);

    // fprintf(out_file, "LEND%p:\nnop\nnop ; condition end\n", node);
    // current_file_p = ftell(out_file);
    // fseek(out_file, insert_else_file_p, 0);
    // WriteLittleInd32(data->rip - insert_end_rip, rip_str);
    // fwrite(rip_str, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);
    WriteLittleInd32(buf->size - insert_end_rip, insert_end_buf_p);
}

void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
    exit(1); // TODO: BAD! return error as bool/int
}

void BytecodeIfelse(Node* node, BytecodeData* data){
    assert(node);
    exit(1); // TODO: BAD! return error as bool/int
}

void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
    fprintf(out_file, "LOOPSTART%p:\n", node);
    ASMParseNode(node->left, vars, out_file, optimize);
    fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    ASMParseNode(node->right, vars, out_file, optimize);
    fprintf(out_file, "jmp LOOPSTART%p\n", node);
    fprintf(out_file, "LOOPEND%p:\n", node);
}

const byte test_if_rax_is_zero[3] = {0x48, 0x85, 0xC0};                // cmp rax, 0
const byte jump_if_equal[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};    // je (place)

void BytecodeLoop(Node* node, BytecodeData* data){
    assert(node);
    CodeBuffer* buf = data->buffer;
    // fprintf(out_file, "LOOPSTART%p:\n", node);
    // byte* loop_start_buf_p = buf->size;
    int loop_start_rip = buf->size;
    BytecodeParseNode(node->left, data);

    // fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    // fwrite(test_if_rax_is_zero, sizeof(char), 3, out_file);
    AddCodeConst(buf, test_if_rax_is_zero);
    // fwrite(jump_if_equal, sizeof(char), 6, out_file);
    AddCodeConst(buf, jump_if_equal);
    byte* insert_loop_end_buf_p = buf->buffer + buf->size - sizeof(int);
    int insert_loop_end_rip = buf->size;

    BytecodeParseNode(node->right, data);
    AddCodeConst(buf, jmp_rel);
    int jmp_to_loop_start_dist = loop_start_rip - buf->size;
    WriteLittleInd32(jmp_to_loop_start_dist, buf->buffer + buf->size - sizeof(int));
    // fprintf(out_file, "LOOPEND%p:\n", node);
    int loop_end_rip = buf->size;
    WriteLittleInd32(loop_end_rip - insert_loop_end_rip, insert_loop_end_buf_p);
    // long current_file_p = ftell(out_file);
    // fseek(out_file, insert_loop_end_file_p, 0);
    // byte jmp_dist_str[4];
    // WriteLittleInd32(data->rip - insert_loop_end_rip, jmp_dist_str);
    // fwrite(jmp_dist_str, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);

}

void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
   // printf("yeah assg\n");
    ASMVarDecl(node, vars, out_file, optimize);
    // ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);
}

void BytecodeAssg(Node* node, BytecodeData* data){
    assert(node);
   // printf("yeah assg\n");
    // ASMParseNode(node->right, vars, out_file);
    BytecodeVarDecl(node, data);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);

}

void ASMCall(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
   // printf("yeah call\n");
    ASMParseNode(node->right, vars, out_file, optimize); // arguments in stack
    if (optimize & 1) {
        for (int i = 0; i < MIN(vars->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = vars->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (vars->array[var_index_sorted].is_arg) {
                var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - vars->arg_cnt;
            }
            fprintf(out_file, "mov [rbp + %d], r%d ; saving optimized vars\n", var_offset * 8, i + 8);
        }
    }
    fprintf(out_file, "push rbp ; save rbp before call\n");
    fprintf(out_file, "call %s ; the call\n", node->left->value.name);
    // here i should clear stack
    int arg_cnt = GetArgCnt(node->right);
    fprintf(out_file, "pop rbp \nadd rsp, %d\n", arg_cnt * 8);
    if (optimize & 1) {
        for (int i = 0; i < MIN(vars->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = vars->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (vars->array[var_index_sorted].is_arg) {
                var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - vars->arg_cnt;
            }
            fprintf(out_file, "mov r%d, [rbp + %d] ; restoring optimized vars\n", i + 8, var_offset * 8);
        }
    }
}

const byte save_rpb[1] = {0x55};                                            // push rbp
const byte call[5] = {0xE8, 0x00, 0x00, 0x00, 0x00};                        // call _func name_
const byte restore_rbp[1] = {0x5D};                                         // pop rbp
const byte clear_arguments[7] = {0x48, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x00}; // add rsp, arg_cnt * 8

const byte nop[1] = {0x90};

void BytecodeCall(Node* node, BytecodeData* data){
    assert(node);
   // printf("yeah call %p\n", node);
    CodeBuffer* buf = data->buffer;
    // fprintf(out_file, "push rbp ; save rbp before call\n");
    // fwrite(save_rpb, sizeof(char), 1, out_file);
    BytecodeParseNode(node->right, data); // args
    if (data->optimize & 1) {
        for (int i = 0; i < MIN(data->variables->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = data->variables->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (data->variables->array[var_index_sorted].is_arg) {
                var_offset = (data->variables->size - data->variables->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - data->variables->arg_cnt;
            }
            //fprintf(out_file, "mov [rbp + %d], r%d ; saving optimized vars\n", var_offset * 8, i + 8);
            OptimSaveReg(buf, i, var_offset * 8);
        }
    }
    AddCodeConst(buf, save_rpb);
    // fprintf(out_file, "call %s ; the call\n", node->left->value.name);
    // fwrite(call, sizeof(char), 5, out_file);
    int insert_function_buf_p = buf->size + 1;
    AddCodeConst(buf, call);
    AddFunctionPos(data->func_needed, node->left->value.name, buf->size, insert_function_buf_p);

    // here i should clear stack
    int arg_cnt = GetArgCnt(node->right) * 8;
    // fprintf(out_file, "add rsp, %d\npop rbp ; after call\n", arg_cnt * 8);
    // fwrite(clear_arguments, sizeof(char), 3, out_file);
    AddCodeConst(buf, restore_rbp);
    AddCodeConst(buf, clear_arguments);
    WriteLittleInd32(arg_cnt, buf->buffer + buf->size - sizeof(int));
    if (data->optimize & 1) {
        for (int i = 0; i < MIN(data->variables->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = data->variables->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (data->variables->array[var_index_sorted].is_arg) {
                var_offset = (data->variables->size - data->variables->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - data->variables->arg_cnt;
            }
            //fprintf(out_file, "mov r%d, [rbp + %d] ; restoring optimized vars\n", i + 8, var_offset * 8);
            OptimRestoreReg(buf, i, var_offset * 8);
        }
    }
}

int GetArgCnt(Node* arg_node) {
    if (arg_node == NULL) {
        return 0;
    }
    assert(arg_node->type == ARG_TYPE);
    return 1 + GetArgCnt(arg_node->right);
}

void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
   // printf("RETURN %p\n", node->right);
    ASMParseNode(node->right, vars, out_file, optimize);
    int var_cnt = (vars->size - vars->arg_cnt) * 8;
    fprintf(out_file, "add rsp, %d\n", var_cnt);
    fprintf(out_file, "ret ; thats it\n");
}

// const byte ret[1] = {0xC3};

void BytecodeReturn(Node* node, BytecodeData* data){
    assert(node);
   // printf("RETURN %p\n", node->right);
    BytecodeParseNode(node->right, data);
    // fprintf(out_file, "ret ; thats it\n");
    // fwrite(ret, sizeof(char), 1, out_file);
    CodeBuffer* buf = data->buffer;
    int var_cnt = (data->variables->size - data->variables->arg_cnt) * 8;
    AddCodeConst(buf, clear_vars);
    // fwrite(var_cnt_str, sizeof(char), 4, out_file);
    WriteLittleInd32(var_cnt, buf->buffer + buf->size - sizeof(int));
    AddCodeConst(buf, ret);
}

void ASMMath(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
    // fprintf(out_file, "mov rax, 0 ; here should be something more intellectual\n");
    ASMParseNode(node->left, vars, out_file, optimize);
    // fprintf(out_file, "mov rbx, rax\n");
    fprintf(out_file, "push rax\n");
    ASMParseNode(node->right, vars, out_file, optimize);
    fprintf(out_file, "mov rbx, rax\n");
    fprintf(out_file, "pop rax\n");
    fprintf(out_file, "xor rdx, rdx\n");
    if (node->value.math <= OP_DIV) {
        switch (node->value.math) {
            case OP_ADD: fprintf(out_file, "add rax, rbx\n"); break;
            case OP_SUB: fprintf(out_file, "sub rax, rbx\n"); break;
            case OP_MUL: fprintf(out_file, "mul rbx\n");      break;
            case OP_DIV: fprintf(out_file, "div rbx\n");      break;
        }
    } else {
        fprintf(out_file, "cmp rax, rbx\n");
        fprintf(out_file, "mov rax, 0\n");
        switch (node->value.math) {
            case OP_EQUAL:   fprintf(out_file, "sete al\n");   break;
            case OP_NE:      fprintf(out_file, "setne al\n");  break;
            case OP_LOE:     fprintf(out_file, "setle al\n");  break;
            case OP_GOE:     fprintf(out_file, "setge al\n");  break;
            case OP_LESS:    fprintf(out_file, "setl al\n");   break;
            case OP_GREATER: fprintf(out_file, "setg al\n");   break;
        }
        fprintf(out_file, "xor rbx, rbx\nmov bl, al\nxor rax, rax\nmov al, bl\n");
    }
}

const byte save_left_value[1] = {0x50};                          // push rax
const byte save_right_value[3] = {0x48, 0x89, 0xC3};             // mov rbx, rax
const byte restore_left_value[1] = {0x58};                       // pop rax
const byte clear_rdx[3] = {0x48, 0x31, 0xD2};                    // xor rdx, rdx
const byte big_clear_rax[5] = {0xB8, 0x00, 0x00, 0x00, 0x00};    // mov rax, 0 (does not changes flags)

const byte math_add[3] = {0x48, 0x01, 0xD8};                     // add rax, rbx
const byte math_sub[3] = {0x48, 0x29, 0xD8};                     // sub rax, rbx
const byte math_mul[3] = {0x48, 0xF7, 0xE3};                     // mul rbx (rdx cleared)
const byte math_div[3] = {0x48, 0xF7, 0xF3};                     // div rbx (rdx cleared)

const byte do_compare[3] = {0x48, 0x39, 0xD8};                   // cmp rax, rbx

const byte cmp_eq[3] = {0x0F, 0x94, 0xC0};                       // sete  al
const byte cmp_ne[3] = {0x0F, 0x95, 0xC0};                       // setne al
const byte cmp_le[3] = {0x0F, 0x9E, 0xC0};                       // setle al
const byte cmp_ge[3] = {0x0F, 0x9D, 0xC0};                       // setge al
const byte cmp_ll[3] = {0x0F, 0x9C, 0xC0};                       // setl  al
const byte cmp_gg[3] = {0x0F, 0x9F, 0xC0};                       // setg  al

void BytecodeMath(Node* node, BytecodeData* data){
    assert(node);
    CodeBuffer* buf = data->buffer;
   // printf("bruhableeeeeeeeeeeee\n");
    BytecodeParseNode(node->left, data);
   // printf("math! left is %p, right is %p\n", node->left, node->right);
    // fprintf(out_file, "push rax\n");
    // fwrite(save_left_value, sizeof(char), 1, out_file);
    AddCodeConst(buf, save_left_value);
    BytecodeParseNode(node->right, data);
    // fprintf(out_file, "mov rbx, rax\n", );
    // fwrite(save_right_value, sizeof(char), 3, out_file);
    AddCodeConst(buf, save_right_value);
    // fprintf(out_file, "pop rax\n");
    // fwrite(restore_left_value, sizeof(char), 1, out_file);
    AddCodeConst(buf, restore_left_value);
    // fprintf(out_file, "xor rdx, rdx\n");
    // fwrite(clear_rdx, sizeof(char), 3, out_file);
    AddCodeConst(buf, clear_rdx);
    if (node->value.math <= OP_DIV) {
        switch (node->value.math) {
            // case OP_ADD: fprintf(out_file, "add rax, rbx\n"); break;
            case OP_ADD: AddCodeConst(buf, math_add); break;
            // case OP_SUB: fprintf(out_file, "sub rax, rbx\n"); break;
            case OP_SUB: AddCodeConst(buf, math_sub); break;
            // case OP_MUL: fprintf(out_file, "mul rbx\n");      break;
            case OP_MUL: AddCodeConst(buf, math_mul); break;
            // case OP_DIV: fprintf(out_file, "div rbx\n");      break;
            case OP_DIV: AddCodeConst(buf, math_div); break;
        }
    } else {
        // fprintf(out_file, "cmp rax, rbx\n");
        // fwrite(do_compare, sizeof(char), 3, out_file);
        AddCodeConst(buf, do_compare);
        AddCodeConst(buf, big_clear_rax);
        switch (node->value.math) {
            // case OP_EQUAL:   fprintf(out_file, "sete al\n");   break;
            case OP_EQUAL :   AddCodeConst(buf, cmp_eq); break;
            // case OP_NE:      fprintf(out_file, "setne al\n");  break;
            case OP_NE :      AddCodeConst(buf, cmp_ne); break;
            // case OP_LOE:     fprintf(out_file, "setle al\n");  break;
            case OP_LOE :     AddCodeConst(buf, cmp_le); break;
            // case OP_GOE:     fprintf(out_file, "setge al\n");  break;
            case OP_GOE :     AddCodeConst(buf, cmp_ge); break;
            // case OP_LESS:    fprintf(out_file, "setl al\n");   break;
            case OP_LESS :    AddCodeConst(buf, cmp_ll); break;
            // case OP_GREATER: fprintf(out_file, "setg al\n");   break;
            case OP_GREATER : AddCodeConst(buf, cmp_gg); break;
        }
        // fprintf(out_file, "xor rbx, rbx\nmov bl, al\nxor rax, rax\nmov al, bl\n");
    }
}

void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file, int optimize){
    assert(node);
   // printf("doing constant %d... %p\n", node->value.num, node);
    fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
}

const byte get_constant[5] = {0xB8, 0x00, 0x00, 0x00, 0x00};

void BytecodeConstant(Node* node, BytecodeData* data){
    assert(node);
   // printf("doing constant %d... %p\n", node->value.num, node);
    CodeBuffer* buf = data->buffer;
    // fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
    // fwrite(get_constant, sizeof(char), 1, out_file);
    AddCodeConst(buf, get_constant);
    // byte const_str[4];
    // WriteLittleInd32(node->value.num, const_str);
    // fwrite(const_str, sizeof(char), 4, out_file);
    WriteLittleInd32(node->value.num, buf->buffer + buf->size - sizeof(int));
}
