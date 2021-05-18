#include "assembly.h"

typedef unsigned char byte;

const int optimized_vars = 4;

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
    //int rip;
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

CodeBuffer* CreateCodeBuffer();
void AddCode(CodeBuffer* buf, const unsigned char* code, int len);
void DestroyCodeBuffer(CodeBuffer* buf);

FuncTable* CreateFuncTable();
void AddFunctionPos(FuncTable* table, char* func_name, uint64_t position);
void DestroyFuncTable(FuncTable* table);

void IncludeStdlib(FILE* out_file);
void IncludeStdlibBin(CodeBuffer* buf, int in_buf, int out_buf, int new_line);

int GetArgCnt(Node* arg_node);
void ASMFdecl(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMName(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMComp(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMCall(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMMath(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);
void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file, bool optimize);

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
    buf->capacity = 0x1000;
    buf->buffer = (byte*)calloc(buf->capacity, sizeof(byte));
    return buf;
}

void AddCode(CodeBuffer* buf, const unsigned char* code, int len) {
    if (len + buf->size > buf->capacity) {
        buf->capacity += 0x1000;
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


void ASMParseNode(Node* node, DynamicArray* vars, FILE* out_file, bool optimize) {
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

const unsigned char  r_flag[4] = {0x04, 0x00, 0x00, 0x00};
const unsigned char re_flag[4] = {0x05, 0x00, 0x00, 0x00};
const unsigned char rw_flag[4] = {0x06, 0x00, 0x00, 0x00};

const unsigned char elf_prog_header_address[3] = {0x00, 0x00, 0x40};
const unsigned char elf_header_size = 0xE8;

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

const unsigned char init_rbp[3] = {0x48, 0x89, 0xE5};  // mov rbp, rsp \n
const unsigned char create_place_for_vars[7] = {0x48, 0x81, 0xEC, 0x00, 0x00, 0x00, 0x00};
const unsigned char clear_vars[7] = {0x48, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x00};
const unsigned char end_proggram[10] = {0xB8, 0x3C, 0x00, 0x00, 0x00, 0x48, 0x31, 0xFF, 0x0F, 0x05};
const unsigned char data_seg[21] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void SetHeadersHeader(ProgramHeader* header) {
    header->flags[0] = 4; //elf_prog_header->flags = r_flag;
    //elf_prog_header->offset = {0x00};
    header->virtual_address[2] = 0x40; //elf_prog_header->virtual_address = {0x00, 0x00, 0x40};
    header->phys_address[2] = 0x40; //elf_prog_header->phys_address = {0x00, 0x00, 0x40};
    header->file_size[0] = elf_header_size;//elf_prog_header->file_size = {0xe8, 0x00};
    header->mem_size[0] = elf_header_size;//elf_prog_header->mem_size = {0xe8, 0x00};
    header->alignment[1] = 0x10;
}

void SetDataHeader(ProgramHeader* header) {
    header->flags[0] = 6;
    header->offset[1] = 0x10;
    header->virtual_address[1] = 0x10;
    header->virtual_address[2] = 0x40;
    header->phys_address[1] = 0x10;
    header->phys_address[2] = 0x40;
    header->file_size[0] = 0x15;
    header->mem_size[0] = 0x15;
    header->alignment[1] = 0x10;
}

void SetCodeHeader(ProgramHeader* header) {
    header->flags[0] = 5; //code_header->flags = re_flag;
    header->offset[1] = 0x20; //e8 + 15
    header->virtual_address[1] = 0x20; //code_header->virtual_address = {0xFD, 0x00, 0x40};
    header->virtual_address[2] = 0x40;
    header->phys_address[1] = 0x20;//code_header->phys_address = {0xFD, 0x00, 0x40};
    header->phys_address[2] = 0x40;
    //code_header->file_size =  strlen(code)
    //code_header->mem_size =   strlen(code)
    header->alignment[1] = 0x10;
}

void Encode(Node* root, const char* out_filename, bool optimize) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");
    ElfHeader* elf_header = CreateElfHeader();

    ProgramHeader* elf_prog_header = CreateProgramHeader();
    SetHeadersHeader(elf_prog_header);

    ProgramHeader* data_header = CreateProgramHeader();
    SetDataHeader(data_header);


    ProgramHeader* code_header = CreateProgramHeader();
    SetCodeHeader(code_header);


    //code_header->alignment = {0x00};



    // fwrite(elf_header, sizeof(char), sizeof(ElfHeader), out_file);
    // fwrite(elf_prog_header, sizeof(char), sizeof(ProgramHeader), out_file);
    // fwrite(data_header, sizeof(char), sizeof(ProgramHeader), out_file);
    // fwrite(code_header, sizeof(char), sizeof(ProgramHeader), out_file);
    // fwrite(data_seg, sizeof(char), 21, out_file);

    //int insert_code_size_place = sizeof(ElfHeader) + 2 * sizeof(ProgramHeader) + 32; // insert twice
    //long stdlib_start_file_p = ftell(out_file);
    //int stdlib_start_rip = (int) stdlib_start_file_p;
    //IncludeStdlibBin(out_file, in_buffer_pos, out_buffer_pos, new_line_pos);
    CodeBuffer* buf = CreateCodeBuffer();
    int stdlib_start       = *((int*)code_header->virtual_address);
    int data_segment_start = *((int*)data_header->virtual_address);
    int  in_buffer_pos     = data_segment_start;
    int out_buffer_pos     = data_segment_start + 1;
    int   new_line_pos     = data_segment_start + 20;
    IncludeStdlibBin(buf, in_buffer_pos, out_buffer_pos, new_line_pos);
    int entry_point = stdlib_start + buf->size;
    WriteLittleInd64(entry_point, elf_header->values + 24);
    //fclose(out_file);
    DynamicArray* current_vars = root->value.variables;
    FuncTable* func_table = CreateFuncTable();
    FuncTable* func_needed = CreateFuncTable();
    // add stdlib functions to func table
    printf("input start: %x\n", stdlib_start);
    AddFunctionPos(func_table, "input", 0, 0);
    AddFunctionPos(func_table, "print", 0x81, 0x81);
    //entry point == right after stdlib == elf header size (const 64) + 3 prog headers size (const 56) + stdlib size
    // WriteEntryPoint(elf_header, 64 + 56 * 3 + stdlib size);
    //long entry_point_file_p = ftell(out_file);
    //WriteEntryPoint(elf_header, entry_point_file_p);
    //long current_file_p = ftell(out_file);
    //fseek(out_file, 24, 0);
    //unsigned char entry_p_str[8];
    //WriteLittleInd64(entry_point_file_p + 0x400000, entry_p_str);
    //fwrite(entry_p_str, sizeof(char), 8, out_file);
    //fseek(out_file, current_file_p, 0);
    BytecodeData data = {current_vars, buf, func_table, func_needed};
    //fprintf(out_file, "sub rsp, %d\n ; 48_81 (83 if imm8 instead of imm32)_\n", current_vars->size * 8);
    //fprintf(out_file, "mov rbp, rsp\n");
    //fwrite(create_place_for_vars, sizeof(char), 3, out_file);
    AddCode(buf, create_place_for_vars, 3);
    // unsigned char var_cnt_str[4];
    int var_cnt = data.variables->size * 8;
    //WriteLittleInd32(var_cnt * 8, var_cnt_str);
    AddCode(buf, (byte*)(&var_cnt), 4);
    //fwrite(var_cnt_str, sizeof(char), 4, out_file);
    AddCode(buf, init_rbp, 3);
    // fwrite(init_rbp, sizeof(char), 3, out_file);
    // data.rip += 3;

    BytecodeParseNode(root, &data);

    //fwrite(clear_vars, sizeof(char), 3, out_file);
    AddCode(buf, clear_vars, 3);
    //fwrite(var_cnt_str, sizeof(char), 4, out_file);
    AddCode(buf, (byte*)(&var_cnt), 4);
    // data.rip += 7;

    //fwrite(end_proggram, sizeof(char), 10, out_file);
    //data.rip += 10;
    AddCode(buf, end_proggram, 10);

    WriteLittleInd64(buf->size, code_header->file_size);
    WriteLittleInd64(buf->size, code_header->mem_size);
    // unsigned char num32[8];
    // current_file_p = ftell(out_file);
    // WriteLittleInd64(current_file_p - stdlib_start_file_p, num32);
    // fseek(out_file, insert_code_size_place, 0);
    // fwrite(num32, sizeof(char), 8, out_file);
    // fwrite(num32, sizeof(char), 8, out_file); // twice because size in file + size in memory
    // fseek(out_file, current_file_p, 0);
    // set functions
    printf("you are here\n");
    for (int i = 0; i < func_needed->size; ++i) {
        printf("%d of %d\n", i + 1, func_needed->size);
        for (int j = 0; j < func_table->size; ++j) {
            printf("need: %s\n this: %s\n", func_needed->positions[i].func_name, func_table->positions[j].func_name);
            if (strcmp(func_needed->positions[i].func_name, func_table->positions[j].func_name) == 0) {
                printf("ya!!! %s\n", func_needed->positions[i].func_name);
                printf("rip of func is %d\n", func_table->positions[j].rip);
                printf("rip of place is %d\n", func_needed->positions[i].rip);
                int distance = func_table->positions[j].rip - func_needed->positions[i].rip;
                printf("distance is: %X\n", distance);
                WriteLittleInd32(distance, buf->buffer + func_needed->positions[i].file_p);
                // fseek(out_file, func_needed->positions[i].file_p, 0);
                // unsigned char dist_str[4];
                // WriteLittleInd32(distance, dist_str);
                // printf("%X%X%X%X\n", (int)(dist_str[0]), (int)(dist_str[1]), (int)(dist_str[2]), (int)(dist_str[3]));
                // fwrite(dist_str, sizeof(char), 4, out_file);
                printf("ya...\n");
                break;
            }
        }
    }

    byte lot_of_zeroes[0x1000];
    memset(lot_of_zeroes, 0, 0x1000);

    fwrite(elf_header, sizeof(char), sizeof(ElfHeader), out_file);
    fwrite(elf_prog_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(data_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(code_header, sizeof(char), sizeof(ProgramHeader), out_file);

    fwrite(lot_of_zeroes, sizeof(byte), 0x1000 - ftell(out_file), out_file); //align
    lot_of_zeroes[0x14] = 0x0a; // new_line
    fwrite(lot_of_zeroes, sizeof(byte), 0x1000, out_file); // aligned data segment
    lot_of_zeroes[0x14] = 0x00; // new_line
    fwrite(buf->buffer, sizeof(byte), buf->capacity, out_file);
    // fwrite(data_seg, sizeof(char), 21, out_file);


    fclose(out_file);
    // TODO: write this num32!!!
}

void Assembly(Node* root, const char* out_filename, bool optimize) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");
    DynamicArray* current_vars = root->value.variables;
    printf("yeah %p\n", current_vars);
    IncludeStdlib(out_file);
    fprintf(out_file, "section .text\nglobal _start\n");
    fprintf(out_file, "_start:\n");
    fprintf(out_file, "sub rsp, %d\n ; 48_81 (83 if imm8 instead of imm32)_\n", current_vars->size * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    ASMParseNode(root, current_vars, out_file, optimize);
    fprintf(out_file, "add rsp, %d\nmov rax, 0x3C\nxor rdi, rdi\nsyscall\n", current_vars->size * 8);
    printf("bruh1\n");
    fclose(out_file);
    printf("bruh2\n");
}

void IncludeStdlibBin(CodeBuffer* buf, int in_buf, int out_buf, int new_line) {
    FILE* bin_stdlib = NULL;
    open_file(&bin_stdlib, "stdlib.bin", "r");
    char* char_buffer = NULL;
    uint64_t buffer_size = 0;
    read_buffer(&char_buffer, &buffer_size, "stdlib.bin", bin_stdlib);
    //file closed

    AddCode(buf, (byte*)(char_buffer), (int)buffer_size);
    // in_buf += 0x400000;
    // out_buf += 0x400000;
    // new_line += 0x400000;
    printf("kek!!! %x\n", in_buf);
    WriteLittleInd64(in_buf,   buf->buffer + 0x10);
    WriteLittleInd32(in_buf,   buf->buffer + 0x22);
    WriteLittleInd64(in_buf,  buf->buffer + 0x3B);
    WriteLittleInd32(in_buf,  buf->buffer + 0x4E);
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
    //printf("%s\n", buffer);
    //fprintf(out_file, "keklol\n");
    //printf("e2\n");
    printf("eeeeee\n");
    fwrite(buffer, sizeof(char), buffer_size, out_file);
    //fnprintf(out_file, buffer_size, "%s\n", buffer);
    free(buffer);
    // fclose(stdlib_file);
}


void ASMFdecl(Node* node, DynamicArray* vars, FILE* out_file, bool optimize) {
    assert(node);
    Node* fname = node->right;
    int arg_cnt = node->value.variables->arg_cnt;
    int var_cnt = node->value.variables->size;
    fprintf(out_file, "jmp %s_END\n", fname->value.name);
    fprintf(out_file, "%s: ;eeee fdecl\n", fname->value.name);
    fprintf(out_file, "sub rsp, %d\n", (var_cnt - arg_cnt) * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    vars = node->value.variables;
    if (optimize) {
        for (int i = 0; i < MIN(vars->size, optimized_vars); ++i) {
            int var_index_sorted = i;
            int var_index_unsorted = vars->array[var_index_sorted].real_index;
            int var_offset = 0;
            if (vars->array[var_index_sorted].is_arg) {
                var_offset = (vars->size - vars->arg_cnt + 2 + var_index_unsorted);
            } else {
                var_offset = var_index_unsorted - vars->arg_cnt;
            }
            //int var_index_sorted = DAfind(vars, node->left->value.name);
            if (optimize) {
                if (var_index_sorted < optimized_vars) {
                    fprintf(out_file, "mov r%d, [rbp + %d]\n", i + 8, var_offset * 8);
                    // return;
                }
            }
        }
    }
    printf("yeah fdecl %p\n", fname);
    // arguments in stack, rbp + x
    assert(node->value.variables);
    ASMParseNode(node->right->left, node->value.variables, out_file, optimize);
    fprintf(out_file, "add rsp, %d ; this was fdecl\n", (var_cnt - arg_cnt) * 8);
    fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
}

const unsigned char jmp_rel[5]  = {0xE9, 0x00, 0x00, 0x00, 0x00};              // jmp _
const unsigned char ret[1] = {0xC3};


void BytecodeFdecl(Node* node, BytecodeData* data) {
    assert(node);
    Node* fname = node->right;
    //save function
    CodeBuffer* buf = data->buffer;
    //--------fprintf(out_file, "jmp %s_END\n", fname->value.name);
    //fwrite(jmp_rel, sizeof(char), 5, out_file);
    AddCode(buf, jmp_rel, 5);
    byte* jmp_buf_pointer = buf->last_command + 1;
    int jmp_rip = buf->size;

    AddFunctionPos(data->func_table, fname->value.name, buf->size, 0);

    //--------fprintf(out_file, "mov rbp, rsp\nadd rbp, 8 ; this was fdecl\n");
    //fwrite(init_rbp, sizeof(char), 3, out_file);
    // fwrite(create_place_for_vars, sizeof(char), 3, out_file);
    AddCode(buf, create_place_for_vars, 3);
    DynamicArray* current_variables = data->variables;
    data->variables = node->value.variables;
    assert(node->value.variables);

    // unsigned char var_cnt_str[4];
    int var_cnt = (data->variables->size - data->variables->arg_cnt) * 8;
    //WriteLittleInd32(var_cnt * 8, var_cnt_str);
    // fwrite(var_cnt_str, sizeof(char), 4, out_file);
    printf("yeah func decl!\n");
    AddCode(buf, (byte*)(&var_cnt), 4);
    AddCode(buf, init_rbp, 3);
    printf("yeah fdecl %p\n", fname);

    // arguments in stack, rbp + x
    BytecodeParseNode(node->right->left, data);
    // fwrite(clear_vars, sizeof(char), 3, out_file);
    AddCode(buf, clear_vars, 3);
    // fwrite(var_cnt_str, sizeof(char), 4, out_file);
    AddCode(buf, (byte*)(&var_cnt), 4);
    data->variables = current_variables;

    //--------fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
    // fwrite(ret, sizeof(char), 1, out_file);
    AddCode(buf, ret, 1);

    //write jump
    int jmp_relative_distance = buf->size - jmp_rip;
    printf("func jump distance: %d\n", jmp_relative_distance);
    // long current_file_p = ftell(out_file);
    WriteLittleInd32(jmp_relative_distance, jmp_buf_pointer);

    // unsigned char number[4];
    // WriteLittleInd32(jmp_relative_distance, number);
    // fseek(out_file, jmp_file_pointer, 0);
    // fwrite(number, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);

}

void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file, bool optimize) {
    assert(node);
    ASMParseNode(node->right, vars, out_file, optimize);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    printf("kk\n\n");
    int var_index_sorted = DAfind(vars, node->left->value.name);
    if (optimize) {
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
    printf("yeah %d * 8\n", var_offset);
    var_offset *= 8;
    fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", var_offset);
}

const unsigned char var_decl_short[4] = {0x48, 0x89, 0x45, 0x00};
const unsigned char var_decl_long[7] = {0x48, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00};

void BytecodeVarDecl(Node* node, BytecodeData* data) {
    assert(node);
    BytecodeParseNode(node->right, data);
    CodeBuffer* buf = data->buffer;
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", node->value.var_number * 8);
    int var_index_sorted = DAfind(data->variables, node->left->value.name);
    int var_index_unsorted = data->variables->array[var_index_sorted].real_index;
    int var_offset = 0;
    if (data->variables->array[var_index_sorted].is_arg) {
        var_offset = (data->variables->size - data->variables->arg_cnt + 2 + var_index_unsorted);
    } else {
        var_offset = var_index_unsorted - data->variables->arg_cnt;
    }
    printf("yeah %d * 8\n", var_offset);
    var_offset *= 8;
    AddCode(buf, var_decl_long, 3);
    AddCode(buf, (byte*)(&var_offset), 4);
    // if (var_offset <= 127) {
    //     char short_var_offset = (var_offset & 0xFF);
    //     fwrite(var_decl_short, sizeof(char), 3, out_file);
    //     fwrite(&short_var_offset, sizeof(char), 1, out_file);
    //     data->rip += 4;
    // } else {
    //     fwrite(var_decl_long, sizeof(char), 3, out_file);
    //     unsigned char var_offset_str[4];
    //     WriteLittleInd32(var_offset, var_offset_str);
    //     fwrite(var_offset_str, sizeof(char), 4, out_file);
    //     data->rip += 7;
    // }

}

void ASMName(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    // i need to find var number and mov it to rax
    int var_index_sorted = DAfind(vars, node->value.name);
    if (optimize) {
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
    printf("name %d\n", var_offset * 8);
    fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", var_offset * 8);

}

const unsigned char var_get_short[4] = {0x48, 0x8B, 0x45, 0x00};
const unsigned char var_get_long[7]  = {0x48, 0x8B, 0x85, 0x00, 0x00, 0x00, 0x00};

void BytecodeName(Node* node, BytecodeData* data) {
    assert(node);
    CodeBuffer* buf = data->buffer;
    printf("name!!!!! %p\n", node);
    // i need to find var number and mov it to rax
    int var_index_sorted = DAfind(data->variables, node->value.name);
    int var_index_unsorted = data->variables->array[var_index_sorted].real_index;
    int var_offset = 0;
    if (data->variables->array[var_index_sorted].is_arg) {
        var_offset = (data->variables->size - data->variables->arg_cnt + 2 + var_index_unsorted);
    } else {
        var_offset = var_index_unsorted - data->variables->arg_cnt;
    }
    printf("yeah %d * 8\n", var_offset);
    var_offset *= 8;
    //fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", DAfind(vars, node->value.name) * 8);
    AddCode(buf, var_get_long, 3);
    AddCode(buf, (byte*)(&var_offset), 4);
    // if (var_offset <= 127) {
    //     char short_var_offset = (var_offset & 0xFF);
    //     fwrite(var_get_short, sizeof(char), 3, out_file);
    //     fwrite(&short_var_offset, sizeof(char), 1, out_file);
    //     data->rip += 4;
    // } else {
    //     fwrite(var_get_long, sizeof(char), 3, out_file);
    //     unsigned char var_offset_str[4];
    //     WriteLittleInd32(var_offset, var_offset_str);
    //     fwrite(var_offset_str, sizeof(char), 4, out_file);
    //     data->rip += 7;
    // }
}

void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    // need to push arguments, but first argument must be pushed last, so parse right son
    if (node->right != NULL) {
        ASMParseNode(node->right, vars, out_file, optimize);
    }
    ASMParseNode(node->left, vars, out_file, optimize);
    fprintf(out_file, "push rax ; this was argument passing\n");
    assert(node);
}

const unsigned char add_argument[1] = {0x50};

void BytecodeArgument(Node* node, BytecodeData* data){
    // need to push arguments, but first argument must be pushed last, so parse right son
    assert(node);
    CodeBuffer* buf = data->buffer;
    if (node->right != NULL) {
        BytecodeParseNode(node->right, data);
    }
    BytecodeParseNode(node->left, data);
    //fprintf(out_file, "push rax ; this was argument passing\n");
    // fwrite(add_argument, sizeof(char), 1, out_file);
    AddCode(buf, add_argument, 1);
}

void ASMComp(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    printf("yeah compound\n");
    ASMParseNode(node->right, vars, out_file, optimize);
}

void BytecodeComp(Node* node, BytecodeData* data){
    assert(node);
    printf("yeah compound\n");
    BytecodeParseNode(node->right, data);
}

void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    printf("yeah statement %p %p\n", node->left, node->right);
    ASMParseNode(node->left, vars, out_file, optimize);
    ASMParseNode(node->right, vars, out_file, optimize);
}

void BytecodeStatement(Node* node, BytecodeData* data){
    assert(node);
    printf("yeah statement %p %p\n", node->left, node->right);
    BytecodeParseNode(node->left, data);
    BytecodeParseNode(node->right, data);
}

void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    ASMParseNode(node->left, vars, out_file, optimize);
    fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    ASMParseNode(node->right->left, vars, out_file, optimize);
    fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    ASMParseNode(node->right->right, vars, out_file, optimize);
    fprintf(out_file, "LEND%p:\nnop\nnop ; condition end\n", node);
}

const unsigned char check_if_cond_true[4] = {0x48, 0x83, 0xF8, 0x00};
const unsigned char jump_to_else[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};

void BytecodeCondition(Node* node, BytecodeData* data){
    assert(node);
    BytecodeParseNode(node->left, data);
    CodeBuffer* buf = data->buffer;
    //fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    // fwrite(check_if_cond_true, sizeof(char), 4, out_file);
    AddCode(buf, check_if_cond_true, 4);
    // fwrite(jump_to_else, sizeof(char), 6, out_file);
    AddCode(buf, jump_to_else, 6);
    int insert_else_rip = buf->size;
    byte* insert_else_buf_p = buf->last_command + 2;
    BytecodeParseNode(node->right->left, data);

    //fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    // fwrite(jmp_rel, sizeof(char), 5, out_file);
    AddCode(buf, jmp_rel, 5);
    int insert_end_rip = buf->size;
    byte* insert_end_buf_p = buf->last_command + 1;
    // now insert data->rip as else
    // long current_file_p = ftell(out_file);
    // long insert_end_file_p = current_file_p - 4;
    // int insert_end_rip = data->rip;
    // fseek(out_file, insert_else_file_p, 0);
    // unsigned char rip_str[4];
    // WriteLittleInd32(data->rip - insert_else_rip, rip_str);
    // fwrite(rip_str, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);
    int jmp_diff = buf->size - insert_else_rip;
    WriteLittleInd32(jmp_diff, insert_else_buf_p);
    BytecodeParseNode(node->right->right, data);

    //fprintf(out_file, "LEND%p:\nnop\nnop ; condition end\n", node);
    // current_file_p = ftell(out_file);
    // fseek(out_file, insert_else_file_p, 0);
    // WriteLittleInd32(data->rip - insert_end_rip, rip_str);
    // fwrite(rip_str, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);
    WriteLittleInd32(buf->size - insert_end_rip, insert_end_buf_p);
}

void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    exit(1);
}

void BytecodeIfelse(Node* node, BytecodeData* data){
    assert(node);
    exit(1);
}

void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    fprintf(out_file, "LOOPSTART%p:\n", node);
    ASMParseNode(node->left, vars, out_file, optimize);
    fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    ASMParseNode(node->right, vars, out_file, optimize);
    fprintf(out_file, "jmp LOOPSTART%p\n", node);
    fprintf(out_file, "LOOPEND%p:\n", node);
}

const unsigned char test_if_rax_is_zero[3] = {0x48, 0x85, 0xC0};
const unsigned char jump_if_equal[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};

void BytecodeLoop(Node* node, BytecodeData* data){
    assert(node);
    CodeBuffer* buf = data->buffer;
    //fprintf(out_file, "LOOPSTART%p:\n", node);
    //byte* loop_start_buf_p = buf->size;
    int loop_start_rip = buf->size;
    BytecodeParseNode(node->left, data);

    //fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    // fwrite(test_if_rax_is_zero, sizeof(char), 3, out_file);
    AddCode(buf, test_if_rax_is_zero, 3);
    //fwrite(jump_if_equal, sizeof(char), 6, out_file);
    AddCode(buf, jump_if_equal, 6);
    byte* insert_loop_end_buf_p = buf->last_command + 2;
    int insert_loop_end_rip = buf->size;

    BytecodeParseNode(node->right, data);
    AddCode(buf, jmp_rel, 5);
    int jmp_to_loop_start_dist = loop_start_rip - buf->size;
    WriteLittleInd32(jmp_to_loop_start_dist, buf->last_command + 1);
    //fprintf(out_file, "LOOPEND%p:\n", node);
    int loop_end_rip = buf->size;
    WriteLittleInd32(loop_end_rip - insert_loop_end_rip, insert_loop_end_buf_p);
    // long current_file_p = ftell(out_file);
    // fseek(out_file, insert_loop_end_file_p, 0);
    // unsigned char jmp_dist_str[4];
    // WriteLittleInd32(data->rip - insert_loop_end_rip, jmp_dist_str);
    // fwrite(jmp_dist_str, sizeof(char), 4, out_file);
    // fseek(out_file, current_file_p, 0);

}

void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    printf("yeah assg\n");
    ASMVarDecl(node, vars, out_file, optimize);
    //ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    //fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);
}

void BytecodeAssg(Node* node, BytecodeData* data){
    assert(node);
    printf("yeah assg\n");
    //ASMParseNode(node->right, vars, out_file);
    BytecodeVarDecl(node, data);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);

}

void ASMCall(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    printf("yeah call\n");
    ASMParseNode(node->right, vars, out_file, optimize); // arguments in stack
    if (optimize) {
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
    if (optimize) {
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

const unsigned char save_rpb[1] = {0x55}; // push rbp
const unsigned char call[5] = {0xE8, 0x00, 0x00, 0x00, 0x00}; // call _func name_
const unsigned char restore_rbp[1] = {0x5D}; // pop rbp
const unsigned char clear_arguments[7] = {0x48, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x00}; // add rsp, arg_cnt * 8

const unsigned char nop[1] = {0x90};

void BytecodeCall(Node* node, BytecodeData* data){
    assert(node);
    printf("yeah call %p\n", node);
    CodeBuffer* buf = data->buffer;
    //fprintf(out_file, "push rbp ; save rbp before call\n");
    // fwrite(save_rpb, sizeof(char), 1, out_file);
    BytecodeParseNode(node->right, data); //args
    AddCode(buf, save_rpb, 1);
    // fprintf(out_file, "call %s ; the call\n", node->left->value.name);
    // fwrite(call, sizeof(char), 5, out_file);
    int insert_function_buf_p = buf->size + 1;
    AddCode(buf, call, 5);
    AddFunctionPos(data->func_needed, node->left->value.name, buf->size, insert_function_buf_p);

    // here i should clear stack
    int arg_cnt = GetArgCnt(node->right) * 8;
    //fprintf(out_file, "add rsp, %d\npop rbp ; after call\n", arg_cnt * 8);
    // fwrite(clear_arguments, sizeof(char), 3, out_file);
    AddCode(buf, restore_rbp, 1);
    AddCode(buf, clear_arguments, 3);
    AddCode(buf, (byte*)(&arg_cnt), 4);
    // unsigned char arg_cnt_str[4];
    // WriteLittleInd32(arg_cnt, arg_cnt_str);
    // fwrite(arg_cnt_str, sizeof(char), 4, out_file);

    // fwrite(restore_rbp, sizeof(char), 1, out_file);
}

int GetArgCnt(Node* arg_node) {
    if (arg_node == NULL) {
        return 0;
    }
    assert(arg_node->type == ARG_TYPE);
    return 1 + GetArgCnt(arg_node->right);
}

void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    printf("RETURN %p\n", node->right);
    ASMParseNode(node->right, vars, out_file, optimize);
    int var_cnt = (vars->size - vars->arg_cnt) * 8;
    fprintf(out_file, "add rsp, %d\n", var_cnt);
    fprintf(out_file, "ret ; thats it\n");
}

//const unsigned char ret[1] = {0xC3};

void BytecodeReturn(Node* node, BytecodeData* data){
    assert(node);
    printf("RETURN %p\n", node->right);
    BytecodeParseNode(node->right, data);
    //fprintf(out_file, "ret ; thats it\n");
    // fwrite(ret, sizeof(char), 1, out_file);
    CodeBuffer* buf = data->buffer;
    int var_cnt = (data->variables->size - data->variables->arg_cnt) * 8;
    AddCode(buf, clear_vars, 3);
    // fwrite(var_cnt_str, sizeof(char), 4, out_file);
    AddCode(buf, (byte*)(&var_cnt), 4);
    AddCode(buf, ret, 1);
}

void ASMMath(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    //fprintf(out_file, "mov rax, 0 ; here should be something more intellectual\n");
    ASMParseNode(node->left, vars, out_file, optimize);
    //fprintf(out_file, "mov rbx, rax\n");
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

const unsigned char save_left_value[1] = {0x50};
const unsigned char save_right_value[3] = {0x48, 0x89, 0xC3};
const unsigned char restore_left_value[1] = {0x58};
const unsigned char clear_rdx[3] = {0x48, 0x31, 0xD2};
const unsigned char big_clear_rax[5] = {0xB8, 0x00, 0x00, 0x00, 0x00};

const unsigned char math_add[3] = {0x48, 0x01, 0xD8};
const unsigned char math_sub[3] = {0x48, 0x29, 0xD8};
const unsigned char math_mul[3] = {0x48, 0xF7, 0xE3};
const unsigned char math_div[3] = {0x48, 0xF7, 0xF3};

const unsigned char do_compare[3] = {0x48, 0x39, 0xD8};

const unsigned char cmp_eq[3] = {0x0F, 0x94, 0xC0};
const unsigned char cmp_ne[3] = {0x0F, 0x95, 0xC0};
const unsigned char cmp_le[3] = {0x0F, 0x9E, 0xC0};
const unsigned char cmp_ge[3] = {0x0F, 0x9D, 0xC0};
const unsigned char cmp_ll[3] = {0x0F, 0x9C, 0xC0};
const unsigned char cmp_gg[3] = {0x0F, 0x9F, 0xC0};

void BytecodeMath(Node* node, BytecodeData* data){
    assert(node);
    CodeBuffer* buf = data->buffer;
    printf("bruhableeeeeeeeeeeee\n");
    BytecodeParseNode(node->left, data);
    printf("math! left is %p, right is %p\n", node->left, node->right);
    //fprintf(out_file, "push rax\n");
    // fwrite(save_left_value, sizeof(char), 1, out_file);
    AddCode(buf, save_left_value, 1);
    BytecodeParseNode(node->right, data);
    //fprintf(out_file, "mov rbx, rax\n", );
    // fwrite(save_right_value, sizeof(char), 3, out_file);
    AddCode(buf, save_right_value, 3);
    //fprintf(out_file, "pop rax\n");
    // fwrite(restore_left_value, sizeof(char), 1, out_file);
    AddCode(buf, restore_left_value, 1);
    //fprintf(out_file, "xor rdx, rdx\n");
    // fwrite(clear_rdx, sizeof(char), 3, out_file);
    AddCode(buf, clear_rdx, 3);
    if (node->value.math <= OP_DIV) {
        switch (node->value.math) {
            //case OP_ADD: fprintf(out_file, "add rax, rbx\n"); break;
            case OP_ADD: AddCode(buf, math_add, 3); break;
            //case OP_SUB: fprintf(out_file, "sub rax, rbx\n"); break;
            case OP_SUB: AddCode(buf, math_sub, 3); break;
            //case OP_MUL: fprintf(out_file, "mul rbx\n");      break;
            case OP_MUL: AddCode(buf, math_mul, 3); break;
            //case OP_DIV: fprintf(out_file, "div rbx\n");      break;
            case OP_DIV: AddCode(buf, math_div, 3); break;
        }
    } else {
        //fprintf(out_file, "cmp rax, rbx\n");
        // fwrite(do_compare, sizeof(char), 3, out_file);
        AddCode(buf, do_compare, 3);
        AddCode(buf, big_clear_rax, 5);
        switch (node->value.math) {
            //case OP_EQUAL:   fprintf(out_file, "sete al\n");   break;
            case OP_EQUAL :   AddCode(buf, cmp_eq, 3); break;
            //case OP_NE:      fprintf(out_file, "setne al\n");  break;
            case OP_NE :      AddCode(buf, cmp_ne, 3); break;
            //case OP_LOE:     fprintf(out_file, "setle al\n");  break;
            case OP_LOE :     AddCode(buf, cmp_le, 3); break;
            //case OP_GOE:     fprintf(out_file, "setge al\n");  break;
            case OP_GOE :     AddCode(buf, cmp_ge, 3); break;
            //case OP_LESS:    fprintf(out_file, "setl al\n");   break;
            case OP_LESS :    AddCode(buf, cmp_ll, 3); break;
            //case OP_GREATER: fprintf(out_file, "setg al\n");   break;
            case OP_GREATER : AddCode(buf, cmp_gg, 3); break;
        }
        //fprintf(out_file, "xor rbx, rbx\nmov bl, al\nxor rax, rax\nmov al, bl\n");
    }
}

void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file, bool optimize){
    assert(node);
    printf("doing constant %d... %p\n", node->value.num, node);
    fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
}

const unsigned char get_constant[5] = {0xB8, 0x00, 0x00, 0x00, 0x00};

void BytecodeConstant(Node* node, BytecodeData* data){
    assert(node);
    printf("doing constant %d... %p\n", node->value.num, node);
    CodeBuffer* buf = data->buffer;
    //fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
    // fwrite(get_constant, sizeof(char), 1, out_file);
    AddCode(buf, get_constant, 1);
    // unsigned char const_str[4];
    // WriteLittleInd32(node->value.num, const_str);
    // fwrite(const_str, sizeof(char), 4, out_file);
    AddCode(buf, (byte*)(&(node->value.num)), 4);
}
