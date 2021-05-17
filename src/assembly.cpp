#include "assembly.h"

struct BytecodeData {
    DynamicArray* variables;
    FuncTable* func_table;
    FuncTable* func_needed;
    int rip;
};

FuncTable* CreateFuncTable();
void AddFunctionPos(FuncTable* table, char* func_name, uint64_t position);
void DestroyFuncTable(FuncTable* table);

void IncludeStdlib(FILE* out_file);
void IncludeStdlibBin(FILE* out_file, int in_buf, int out_buf, int new_line);

int GetArgCnt(Node* arg_node);
void ASMFdecl(Node* node, DynamicArray* vars, FILE* out_file);
void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file);
void ASMName(Node* node, DynamicArray* vars, FILE* out_file);
void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file);
void ASMComp(Node* node, DynamicArray* vars, FILE* out_file);
void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file);
void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file);
void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file);
void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file);
void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file);
void ASMCall(Node* node, DynamicArray* vars, FILE* out_file);
void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file);
void ASMMath(Node* node, DynamicArray* vars, FILE* out_file);
void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file);

void BytecodeFdecl(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeVarDecl(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeName(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeArgument(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeComp(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeStatement(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeCondition(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeIfelse(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeLoop(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeAssg(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeCall(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeReturn(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeMath(Node* node, BytecodeData* data, FILE* out_file);
void BytecodeConstant(Node* node, BytecodeData* data, FILE* out_file);



void ASMParseNode(Node* node, DynamicArray* vars, FILE* out_file) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case D_TYPE:    ASMFdecl    (node, vars, out_file);   break;
        case DECL_TYPE: ASMVarDecl  (node, vars, out_file);   break;
        case ID_TYPE:   ASMName     (node, vars, out_file);   break;
        case ARG_TYPE:  ASMArgument (node, vars, out_file);   break;
        case COMP_TYPE: ASMComp     (node, vars, out_file);   break;
        case STAT_TYPE: ASMStatement(node, vars, out_file);   break;
        case COND_TYPE: ASMCondition(node, vars, out_file);   break;
        case IFEL_TYPE: ASMIfelse   (node, vars, out_file);   break;
        case LOOP_TYPE: ASMLoop     (node, vars, out_file);   break;
        case ASSG_TYPE: ASMAssg     (node, vars, out_file);   break;
        case CALL_TYPE: ASMCall     (node, vars, out_file);   break;
        case JUMP_TYPE: ASMReturn   (node, vars, out_file);   break;
        case MATH_TYPE: ASMMath     (node, vars, out_file);   break;
        case NUMB_TYPE: ASMConstant (node, vars, out_file);   break;
    }
}

void BytecodeParseNode(Node* node, BytecodeData* data, FILE* out_file) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case D_TYPE:    BytecodeFdecl    (node, data, out_file);   break;
        case DECL_TYPE: BytecodeVarDecl  (node, data, out_file);   break;
        case ID_TYPE:   BytecodeName     (node, data, out_file);   break;
        case ARG_TYPE:  BytecodeArgument (node, data, out_file);   break;
        case COMP_TYPE: BytecodeComp     (node, data, out_file);   break;
        case STAT_TYPE: BytecodeStatement(node, data, out_file);   break;
        case COND_TYPE: BytecodeCondition(node, data, out_file);   break;
        case IFEL_TYPE: BytecodeIfelse   (node, data, out_file);   break;
        case LOOP_TYPE: BytecodeLoop     (node, data, out_file);   break;
        case ASSG_TYPE: BytecodeAssg     (node, data, out_file);   break;
        case CALL_TYPE: BytecodeCall     (node, data, out_file);   break;
        case JUMP_TYPE: BytecodeReturn   (node, data, out_file);   break;
        case MATH_TYPE: BytecodeMath     (node, data, out_file);   break;
        case NUMB_TYPE: BytecodeConstant (node, data, out_file);   break;
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

void Encode(Node* root, const char* out_filename) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");
    ElfHeader* elf_header = CreateElfHeader();

    ProgramHeader* elf_prog_header = CreateProgramHeader();
    elf_prog_header->flags[0] = 4; //elf_prog_header->flags = r_flag;
    //elf_prog_header->offset = {0x00};
    elf_prog_header->virtual_address[2] = 0x40; //elf_prog_header->virtual_address = {0x00, 0x00, 0x40};
    elf_prog_header->phys_address[2] = 0x40; //elf_prog_header->phys_address = {0x00, 0x00, 0x40};
    elf_prog_header->file_size[0] = elf_header_size;//elf_prog_header->file_size = {0xe8, 0x00};
    elf_prog_header->mem_size[0] = elf_header_size;//elf_prog_header->mem_size = {0xe8, 0x00};
    //elf_prog_header->alignment = {0x00};

    const int in_buffer_pos = 64 + 56 * 3;
    const int out_buffer_pos = in_buffer_pos + 1;
    const int new_line_pos = out_buffer_pos + 19;

    ProgramHeader* data_header = CreateProgramHeader();
    data_header->flags[0] = 6;//data->flags = rw_flag;
    data_header->offset[0] = elf_header_size;//data_header->offset = {0xE8};
    data_header->virtual_address[0] = elf_header_size;//code_header->virtual_address = {0xE8, 0x00, 0x40};
    data_header->virtual_address[2] = 0x40;
    data_header->phys_address[0] = elf_header_size;//code_header->phys_address = {0xE8, 0x00, 0x40};
    data_header->phys_address[2] = 0x40;
    data_header->file_size[0] = 0x15;
    data_header->mem_size[0] = 0x15;
    //code_header->alignment = {0x00};

    ProgramHeader* code_header = CreateProgramHeader();
    code_header->flags[0] = 5; //code_header->flags = re_flag;
    code_header->offset[0] = 0xFD; //e8 + 15
    code_header->virtual_address[0] = 0xFD; //code_header->virtual_address = {0xFD, 0x00, 0x40};
    code_header->virtual_address[2] = 0x40;
    code_header->phys_address[0] = 0xFD;//code_header->phys_address = {0xFD, 0x00, 0x40};
    code_header->phys_address[2] = 0x40;
    //code_header->file_size =  strlen(code)
    //code_header->mem_size =   strlen(code)
    const int code_size_place = 64 + 56 * 2 + 32;
    //code_header->alignment = {0x00};



    fwrite(elf_header, sizeof(char), sizeof(ElfHeader), out_file);
    fwrite(elf_prog_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(data_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(code_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(data_seg, sizeof(char), 21, out_file);
    int insert_code_size_place = sizeof(ElfHeader) + 2 * sizeof(ProgramHeader) + 32; // insert twice
    long stdlib_start_file_p = ftell(out_file);
    int stdlib_start_rip = (int) stdlib_start_file_p;
    IncludeStdlibBin(out_file, in_buffer_pos, out_buffer_pos, new_line_pos);
    //fclose(out_file);
    DynamicArray* current_vars = root->value.variables;
    FuncTable* func_table = CreateFuncTable();
    FuncTable* func_needed = CreateFuncTable();
    // add stdlib functions to func table
    AddFunctionPos(func_table, "input", stdlib_start_rip, stdlib_start_file_p);
    AddFunctionPos(func_table, "print", stdlib_start_rip + 0x4D, stdlib_start_file_p + 0x4D);
    //make elf header and other headers
    //entry point == right after stdlib == elf header size (const 64) + 3 prog headers size (const 56) + stdlib size
    // WriteEntryPoint(elf_header, 64 + 56 * 3 + stdlib size);
    long entry_point_file_p = ftell(out_file);
    //WriteEntryPoint(elf_header, entry_point_file_p);
    long current_file_p = ftell(out_file);
    fseek(out_file, 24, 0);
    unsigned char entry_p_str[8];
    WriteLittleInd64(entry_point_file_p + 0x400000, entry_p_str);
    fwrite(entry_p_str, sizeof(char), 8, out_file);
    fseek(out_file, current_file_p, 0);
    BytecodeData data = {current_vars, func_table, func_needed, entry_point_file_p};
    //fprintf(out_file, "sub rsp, %d\n ; 48_81 (83 if imm8 instead of imm32)_\n", current_vars->size * 8);
    //fprintf(out_file, "mov rbp, rsp\n");
    fwrite(create_place_for_vars, sizeof(char), 3, out_file);
    unsigned char var_cnt_str[4];
    int var_cnt = data.variables->size;
    WriteLittleInd32(var_cnt * 8, var_cnt_str);
    fwrite(var_cnt_str, sizeof(char), 4, out_file);
    data.rip += 7;
    fwrite(init_rbp, sizeof(char), 3, out_file);
    data.rip += 3;

    BytecodeParseNode(root, &data, out_file);

    fwrite(clear_vars, sizeof(char), 3, out_file);
    fwrite(var_cnt_str, sizeof(char), 4, out_file);
    data.rip += 7;

    fwrite(end_proggram, sizeof(char), 10, out_file);
    data.rip += 10;

    unsigned char num32[8];
    current_file_p = ftell(out_file);
    WriteLittleInd64(current_file_p - stdlib_start_file_p, num32);
    fseek(out_file, insert_code_size_place, 0);
    fwrite(num32, sizeof(char), 8, out_file);
    fwrite(num32, sizeof(char), 8, out_file); // twice because size in file + size in memory
    fseek(out_file, current_file_p, 0);
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
                fseek(out_file, func_needed->positions[i].file_p, 0);
                unsigned char dist_str[4];
                WriteLittleInd32(distance, dist_str);
                printf("%X%X%X%X\n", (int)(dist_str[0]), (int)(dist_str[1]), (int)(dist_str[2]), (int)(dist_str[3]));
                fwrite(dist_str, sizeof(char), 4, out_file);
                printf("ya...\n");
                break;
            }
        }
    }

    fseek(out_file, current_file_p, 0);
    fclose(out_file);
    // TODO: write this num32!!!
}

void Assembly(Node* root, const char* out_filename) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");
    DynamicArray* current_vars = root->value.variables;
    printf("yeah %p\n", current_vars);
    IncludeStdlib(out_file);
    fprintf(out_file, "section .text\nglobal _start\n");
    fprintf(out_file, "_start:\n");
    fprintf(out_file, "sub rsp, %d\n ; 48_81 (83 if imm8 instead of imm32)_\n", current_vars->size * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    ASMParseNode(root, current_vars, out_file);
    fprintf(out_file, "add rsp, %d\nmov rax, 0x3C\nxor rdi, rdi\nsyscall\n", current_vars->size * 8);
    printf("bruh1\n");
    fclose(out_file);
    printf("bruh2\n");
}

void IncludeStdlibBin(FILE* out_file, int in_buf, int out_buf, int new_line) {
    FILE* bin_stdlib = NULL;
    open_file(&bin_stdlib, "stdlib.bin", "r");
    char* char_buffer = NULL;
    uint64_t buffer_size = 0;
    read_buffer(&char_buffer, &buffer_size, "stdlib.bin", bin_stdlib);
    //file closed
    unsigned char* buffer = (unsigned char*) char_buffer;
    in_buf += 0x400000;
    out_buf += 0x400000;
    new_line += 0x400000;
    WriteLittleInd64(in_buf, buffer + 0x10);
    WriteLittleInd32(in_buf, buffer + 0x23);
    WriteLittleInd32(out_buf, buffer + 0x78);
    WriteLittleInd64(out_buf, buffer + 0x94);
    WriteLittleInd32(new_line, buffer + 0xAD);
    fwrite(buffer, sizeof(char), buffer_size, out_file);
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


void ASMFdecl(Node* node, DynamicArray* vars, FILE* out_file) {
    assert(node);
    Node* fname = node->right;
    fprintf(out_file, "jmp %s_END\n", fname->value.name);
    fprintf(out_file, "%s:\n", fname->value.name);
    fprintf(out_file, "mov rbp, rsp\n");
    int arg_cnt = node->value.variables->arg_cnt;
    int var_cnt = node->value.variables->size;
    fprintf(stderr, "sub rbp, %d ; this was fdecl\n", var_cnt - arg_cnt);
    printf("yeah fdecl %p\n", fname);
    // arguments in stack, rbp + x
    assert(node->value.variables);
    ASMParseNode(node->right->left, node->value.variables, out_file);
    fprintf(out_file, "add rbp, %d ; this was fdecl\n", var_cnt - arg_cnt);
    fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
}

const unsigned char jmp_rel[5]  = {0xE9, 0x00, 0x00, 0x00, 0x00};              // jmp _
const unsigned char ret[1] = {0xC3};


void BytecodeFdecl(Node* node, BytecodeData* data, FILE* out_file) {
    assert(node);
    Node* fname = node->right;
    //save function
    long jmp_file_pointer = ftell(out_file) + 1;
    //--------fprintf(out_file, "jmp %s_END\n", fname->value.name);
    fwrite(jmp_rel, sizeof(char), 5, out_file);
    data->rip += 5;
    int jmp_rip = data->rip;

    AddFunctionPos(data->func_table, fname->value.name, data->rip, ftell(out_file));

    //--------fprintf(out_file, "mov rbp, rsp\nadd rbp, 8 ; this was fdecl\n");
    fwrite(init_rbp, sizeof(char), 3, out_file);
    data->rip += 3;
    fwrite(create_place_for_vars, sizeof(char), 3, out_file);
    DynamicArray* current_variables = data->variables;
    data->variables = node->value.variables;
    assert(node->value.variables);

    unsigned char var_cnt_str[4];
    int var_cnt = data->variables->size - data->variables->arg_cnt;
    WriteLittleInd32(var_cnt * 8, var_cnt_str);
    fwrite(var_cnt_str, sizeof(char), 4, out_file);
    data->rip += 7;
    printf("yeah fdecl %p\n", fname);

    // arguments in stack, rbp + x
    BytecodeParseNode(node->right->left, data, out_file);
    fwrite(clear_vars, sizeof(char), 3, out_file);
    fwrite(var_cnt_str, sizeof(char), 4, out_file);
    data->rip += 7;
    data->variables = current_variables;

    //--------fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
    fwrite(ret, sizeof(char), 1, out_file);
    data->rip += 1;

    //write jump
    int jmp_relative_distance = data->rip - jmp_rip;
    printf("func jump distance: %d\n", jmp_relative_distance);
    long current_file_p = ftell(out_file);
    unsigned char number[4];
    WriteLittleInd32(jmp_relative_distance, number);
    fseek(out_file, jmp_file_pointer, 0);
    fwrite(number, sizeof(char), 4, out_file);
    fseek(out_file, current_file_p, 0);

}

void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file) {
    assert(node);
    ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    int var_index = node->value.var_number;
    int var_offset = 0;
    if (var_index < vars->arg_cnt) {
        var_offset = (vars->size - vars->arg_cnt + 1 + var_index);
    } else {
        var_offset = var_index - vars->arg_cnt;
    }
    var_offset *= 8;
    fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", var_offset * 8);
}

const unsigned char var_decl_short[4] = {0x48, 0x89, 0x45, 0x00};
const unsigned char var_decl_long[7] = {0x48, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00};

void BytecodeVarDecl(Node* node, BytecodeData* data, FILE* out_file) {
    assert(node);
    BytecodeParseNode(node->right, data, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", node->value.var_number * 8);
    int var_index = node->value.var_number;
    int var_offset = 0;
    if (var_index < data->variables->arg_cnt) {
        var_offset = (data->variables->size - data->variables->arg_cnt + 1 + var_index);
    } else {
        var_offset = var_index - data->variables->arg_cnt;
    }
    printf("EEEEEEEEEEE %d\n", var_offset);
    var_offset *= 8;
    if (var_offset <= 127) {
        char short_var_offset = (var_offset & 0xFF);
        fwrite(var_decl_short, sizeof(char), 3, out_file);
        fwrite(&short_var_offset, sizeof(char), 1, out_file);
        data->rip += 4;
    } else {
        fwrite(var_decl_long, sizeof(char), 3, out_file);
        unsigned char var_offset_str[4];
        WriteLittleInd32(var_offset, var_offset_str);
        fwrite(var_offset_str, sizeof(char), 4, out_file);
        data->rip += 7;
    }

}

void ASMName(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    // i need to find var number and mov it to rax
    fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", DAfind(vars, node->value.name) * 8);

}

const unsigned char var_get_short[4] = {0x48, 0x8B, 0x45, 0x00};
const unsigned char var_get_long[7]  = {0x48, 0x8B, 0x85, 0x00, 0x00, 0x00, 0x00};

void BytecodeName(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    // i need to find var number and mov it to rax
    int var_offset = DAfind(data->variables, node->value.name) * 8;
    //fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", DAfind(vars, node->value.name) * 8);
    if (var_offset <= 127) {
        char short_var_offset = (var_offset & 0xFF);
        fwrite(var_get_short, sizeof(char), 3, out_file);
        fwrite(&short_var_offset, sizeof(char), 1, out_file);
        data->rip += 4;
    } else {
        fwrite(var_get_long, sizeof(char), 3, out_file);
        unsigned char var_offset_str[4];
        WriteLittleInd32(var_offset, var_offset_str);
        fwrite(var_offset_str, sizeof(char), 4, out_file);
        data->rip += 7;
    }
}

void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file){
    // need to push arguments, but first argument must be pushed last, so parse right son
    if (node->right != NULL) {
        ASMParseNode(node->right, vars, out_file);
    }
    ASMParseNode(node->left, vars, out_file);
    fprintf(out_file, "push rax ; this was argument passing\n");
    assert(node);
}

const unsigned char add_argument[1] = {0x50};

void BytecodeArgument(Node* node, BytecodeData* data, FILE* out_file){
    // need to push arguments, but first argument must be pushed last, so parse right son
    if (node->right != NULL) {
        BytecodeParseNode(node->right, data, out_file);
    }
    BytecodeParseNode(node->left, data, out_file);
    //fprintf(out_file, "push rax ; this was argument passing\n");
    fwrite(add_argument, sizeof(char), 1, out_file);
    data->rip++;
    assert(node);
}

void ASMComp(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah compound\n");
    ASMParseNode(node->right, vars, out_file);
}

void BytecodeComp(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    printf("yeah compound\n");
    BytecodeParseNode(node->right, data, out_file);
}

void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah statement %p %p\n", node->left, node->right);
    ASMParseNode(node->left, vars, out_file);
    ASMParseNode(node->right, vars, out_file);
}

void BytecodeStatement(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    printf("yeah statement %p %p\n", node->left, node->right);
    BytecodeParseNode(node->left, data, out_file);
    BytecodeParseNode(node->right, data, out_file);
}

void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    ASMParseNode(node->left, vars, out_file);
    fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    ASMParseNode(node->right->left, vars, out_file);
    fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    ASMParseNode(node->right->right, vars, out_file);
    fprintf(out_file, "LEND%p:\nnop\nnop ; condition end\n", node);
}

const unsigned char check_if_cond_true[4] = {0x48, 0x83, 0xF8, 0x00};
const unsigned char jump_to_else[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};

void BytecodeCondition(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    BytecodeParseNode(node->left, data, out_file);
    //fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    fwrite(check_if_cond_true, sizeof(char), 4, out_file);
    data->rip += 4;
    fwrite(jump_to_else, sizeof(char), 6, out_file);
    int insert_else_rip = data->rip + 6;
    long insert_else_file_p = ftell(out_file) - 4;
    data->rip += 6;
    BytecodeParseNode(node->right->left, data, out_file);

    //fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    fwrite(jmp_rel, sizeof(char), 5, out_file);
    data->rip += 5;
    // now insert data->rip as else
    long current_file_p = ftell(out_file);
    long insert_end_file_p = current_file_p - 4;
    int insert_end_rip = data->rip;
    fseek(out_file, insert_else_file_p, 0);
    unsigned char rip_str[4];
    WriteLittleInd32(data->rip - insert_else_rip, rip_str);
    fwrite(rip_str, sizeof(char), 4, out_file);
    fseek(out_file, current_file_p, 0);
    BytecodeParseNode(node->right->right, data, out_file);

    //fprintf(out_file, "LEND%p:\nnop\nnop ; condition end\n", node);
    current_file_p = ftell(out_file);
    fseek(out_file, insert_else_file_p, 0);
    WriteLittleInd32(data->rip - insert_end_rip, rip_str);
    fwrite(rip_str, sizeof(char), 4, out_file);
    fseek(out_file, current_file_p, 0);
}

void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    exit(1);
}

void BytecodeIfelse(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    exit(1);
}

void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    fprintf(out_file, "LOOPSTART%p:\n", node);
    ASMParseNode(node->left, vars, out_file);
    fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    ASMParseNode(node->right, vars, out_file);
    fprintf(out_file, "LOOPEND%p:\n", node);
}

const unsigned char test_if_rax_is_zero[3] = {0x48, 0x85, 0xC0};
const unsigned char jump_if_equal[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};

void BytecodeLoop(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    //fprintf(out_file, "LOOPSTART%p:\n", node);
    long loop_start_file_p = ftell(out_file);
    int loop_start_rip = data->rip;
    BytecodeParseNode(node->left, data, out_file);

    //fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    fwrite(test_if_rax_is_zero, sizeof(char), 3, out_file);
    data->rip += 3;
    long insert_loop_end_file_p = ftell(out_file) + 2;
    fwrite(jump_if_equal, sizeof(char), 6, out_file);
    data->rip += 6;
    int insert_loop_end_rip = data->rip;

    BytecodeParseNode(node->right, data, out_file);
    //fprintf(out_file, "LOOPEND%p:\n", node);
    int loop_end_rip = data->rip;
    long current_file_p = ftell(out_file);
    fseek(out_file, insert_loop_end_file_p, 0);
    unsigned char jmp_dist_str[4];
    WriteLittleInd32(data->rip - insert_loop_end_rip, jmp_dist_str);
    fwrite(jmp_dist_str, sizeof(char), 4, out_file);
    fseek(out_file, current_file_p, 0);
}

void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah assg\n");
    ASMVarDecl(node, vars, out_file);
    //ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    //fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);
}

void BytecodeAssg(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    printf("yeah assg\n");
    //ASMParseNode(node->right, vars, out_file);
    BytecodeVarDecl(node, data, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);

}

void ASMCall(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah call\n");
    fprintf(out_file, "push rbp ; save rbp before call\n");
    ASMParseNode(node->right, vars, out_file); // arguments in stack
    fprintf(out_file, "call %s ; the call\n", node->left->value.name);
    // here i should clear stack
    int arg_cnt = GetArgCnt(node->right);
    fprintf(out_file, "add rsp, %d\npop rbp ; after call\n", arg_cnt * 8);
}

const unsigned char save_rpb[1] = {0x55}; // push rbp
const unsigned char call[5] = {0xE8, 0x00, 0x00, 0x00, 0x00}; // call _func name_
const unsigned char restore_rbp[1] = {0x5D}; // pop rbp
const unsigned char clear_arguments[7] = {0x48, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x00}; // add rsp, arg_cnt * 8

void BytecodeCall(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    printf("yeah call\n");
    //fprintf(out_file, "push rbp ; save rbp before call\n");
    fwrite(save_rpb, sizeof(char), 1, out_file);
    data->rip += 1;
    BytecodeParseNode(node->right, data, out_file); // arguments in stack
    // fprintf(out_file, "call %s ; the call\n", node->left->value.name);
    long insert_function_file_p = ftell(out_file) + 1;
    fwrite(call, sizeof(char), 5, out_file);
    data->rip += 5;
    AddFunctionPos(data->func_needed, node->left->value.name, data->rip, insert_function_file_p);

    // here i should clear stack
    int arg_cnt = GetArgCnt(node->right);
    //fprintf(out_file, "add rsp, %d\npop rbp ; after call\n", arg_cnt * 8);
    fwrite(clear_arguments, sizeof(char), 3, out_file);
    unsigned char arg_cnt_str[4];
    WriteLittleInd32(arg_cnt, arg_cnt_str);
    fwrite(arg_cnt_str, sizeof(char), 4, out_file);
    data->rip += 7;

    fwrite(restore_rbp, sizeof(char), 1, out_file);
    data->rip += 1;
}

int GetArgCnt(Node* arg_node) {
    if (arg_node == NULL) {
        return 0;
    }
    assert(arg_node->type == ARG_TYPE);
    return 1 + GetArgCnt(arg_node->right);
}

void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("RETURN %p\n", node->right);
    ASMParseNode(node->right, vars, out_file);
    fprintf(out_file, "ret ; thats it\n");
}

//const unsigned char ret[1] = {0xC3};

void BytecodeReturn(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    printf("RETURN %p\n", node->right);
    BytecodeParseNode(node->right, data, out_file);
    //fprintf(out_file, "ret ; thats it\n");
    fwrite(ret, sizeof(char), 1, out_file);
    data->rip += 1;
}

void ASMMath(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    //fprintf(out_file, "mov rax, 0 ; here should be something more intellectual\n");
    ASMParseNode(node->left, vars, out_file);
    //fprintf(out_file, "mov rbx, rax\n");
    fprintf(out_file, "push rax\n");
    ASMParseNode(node->right, vars, out_file);
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

void BytecodeMath(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    BytecodeParseNode(node->left, data, out_file);
    //fprintf(out_file, "push rax\n");
    fwrite(save_left_value, sizeof(char), 1, out_file);
    data->rip += 1;
    BytecodeParseNode(node->right, data, out_file);
    //fprintf(out_file, "mov rbx, rax\n", );
    fwrite(save_right_value, sizeof(char), 3, out_file);
    data->rip += 3;
    //fprintf(out_file, "pop rax\n");
    fwrite(restore_left_value, sizeof(char), 1, out_file);
    data->rip += 1;
    //fprintf(out_file, "xor rdx, rdx\n");
    fwrite(clear_rdx, sizeof(char), 3, out_file);
    data->rip += 3;
    if (node->value.math <= OP_DIV) {
        switch (node->value.math) {
            //case OP_ADD: fprintf(out_file, "add rax, rbx\n"); break;
            case OP_ADD: fwrite(math_add, sizeof(char), 3, out_file); break;
            //case OP_SUB: fprintf(out_file, "sub rax, rbx\n"); break;
            case OP_SUB: fwrite(math_sub, sizeof(char), 3, out_file); break;
            //case OP_MUL: fprintf(out_file, "mul rbx\n");      break;
            case OP_MUL: fwrite(math_mul, sizeof(char), 3, out_file); break;
            //case OP_DIV: fprintf(out_file, "div rbx\n");      break;
            case OP_DIV: fwrite(math_div, sizeof(char), 3, out_file); break;
        }
    } else {
        //fprintf(out_file, "cmp rax, rbx\n");
        fwrite(do_compare, sizeof(char), 3, out_file);
        data->rip += 3;
        switch (node->value.math) {
            //case OP_EQUAL:   fprintf(out_file, "sete al\n");   break;
            case OP_EQUAL :   fwrite(cmp_eq, sizeof(char), 3, out_file); break;
            //case OP_NE:      fprintf(out_file, "setne al\n");  break;
            case OP_NE :      fwrite(cmp_ne, sizeof(char), 3, out_file); break;
            //case OP_LOE:     fprintf(out_file, "setle al\n");  break;
            case OP_LOE :     fwrite(cmp_le, sizeof(char), 3, out_file); break;
            //case OP_GOE:     fprintf(out_file, "setge al\n");  break;
            case OP_GOE :     fwrite(cmp_ge, sizeof(char), 3, out_file); break;
            //case OP_LESS:    fprintf(out_file, "setl al\n");   break;
            case OP_LESS :    fwrite(cmp_ll, sizeof(char), 3, out_file); break;
            //case OP_GREATER: fprintf(out_file, "setg al\n");   break;
            case OP_GREATER : fwrite(cmp_gg, sizeof(char), 3, out_file); break;
        }
        //fprintf(out_file, "xor rbx, rbx\nmov bl, al\nxor rax, rax\nmov al, bl\n");
    }
    data->rip += 3;
}

void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("doing constant %d... %p\n", node->value.num, node);
    fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
}

const unsigned char get_constant[5] = {0xB8, 0x00, 0x00, 0x00, 0x00};

void BytecodeConstant(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    printf("doing constant %d... %p\n", node->value.num, node);
    //fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
    fwrite(get_constant, sizeof(char), 1, out_file);
    unsigned char const_str[4];
    WriteLittleInd32(node->value.num, const_str);
    fwrite(const_str, sizeof(char), 4, out_file);
    data->rip += 5;
}
