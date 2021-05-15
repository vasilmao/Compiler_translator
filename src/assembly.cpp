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
int GetArgCnt(Node* arg_node);
void ASMReturn(Node* node, DynamicArray* vars, FILE* out_file);
void ASMMath(Node* node, DynamicArray* vars, FILE* out_file);
void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file);



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

void AddFunctionPos(FuncTable* table, char* func_name, int position) {
    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->positions = (FunctionPos*)realloc(table->positions, table->capacity * sizeof(FunctionPos));
    }
    int func_name_len = strlen(func_name);
    table->positions[table->size].func_name = (char*)calloc(func_name_len + 1, sizeof(char));
    strncpy(table->positions[table->size].func_name, func_name, func_name_len);
    table->positions[table->size].pos = position;
    table->size++;
}

void DestroyFuncTable(FuncTable* table) {
    for (int i = 0; i < table->size; ++i) {
        free(table->positions[i].func_name);
    }
    free(table->positions);
    free(table);
}

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

    const int in_buffer_pos = 64 + 56 * 3;
    const int out_buffer_pos = in_buffer_pos + 1;
    const int new_line_pos = out_buffer_pos + 19;

    fwrite(elf_header, sizeof(char), sizeof(ElfHeader), out_file);
    fwrite(elf_prog_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(data_header, sizeof(char), sizeof(ProgramHeader), out_file);
    fwrite(code_header, sizeof(char), sizeof(ProgramHeader), out_file);
    IncludeStdlibBin(out_file, in_buffer_pos, out_buffer_pos, new_line_pos);
    //fclose(out_file);
    DynamicArray* current_vars = root->value.variables;
    FuncTable* func_table = CreateFuncTable();
    FuncTable* func_needed = CreateFuncTable();
    //make elf header and other headers
    //make _start and set entry point
    //entry point == right after stdlib == elf header size (const 64) + 3 prog headers size (const 56) + stdlib size (const 190)
    WriteEntryPoint(elf_header, 64 + 56 * 3 + 190);
    BytecodeData data = {current_vars, func_table, func_needed, 64 + 56 * 3 + 190};
    BytecodeParseNode(root, &data, out_file);
    char num32[8];
    WriteLittleInd64(data.rip - (64 + 56 * 3 + 190), num32);
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
    fprintf(out_file, "mov rbp, rsp\nadd rbp, 8 ; this was fdecl\n");
    printf("yeah fdecl %p\n", fname);
    // arguments in stack, rbp + x
    assert(node->value.variables);
    ASMParseNode(node->right->left, node->value.variables, out_file);
    fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
}

const char jmp_rel[5]  = {0xE9, 0x00, 0x00, 0x00, 0x00};              // jmp _
const char init_rbp[7] = {0x48, 0x89, 0xE5, 0x48, 0x83, 0xC5, 0x08};  // mov rbp, rsp \n add rbp, 8
const char ret[1] = {0xC3};


void BytecodeFdecl(Node* node, BytecodeData* data, FILE* out_file) {
    assert(node);
    Node* fname = node->right;
    //save function
    long jmp_file_pointer = ftell(out_file) + 1;
    //--------fprintf(out_file, "jmp %s_END\n", fname->value.name);
    fwrite(jmp_rel, sizeof(char), 5, out_file);
    data->rip += 5;
    int jmp_rip = data->rip;

    AddFunctionPos(data->func_table, fname->value.name, data->rip);

    //--------fprintf(out_file, "mov rbp, rsp\nadd rbp, 8 ; this was fdecl\n");
    fwrite(init_rbp, sizeof(char), 7, out_file);
    data->rip += 7;
    printf("yeah fdecl %p\n", fname);

    // arguments in stack, rbp + x
    assert(node->value.variables);
    DynamicArray* current_variables = data->variables;
    data->variables = node->value.variables;
    BytecodeParseNode(node->right->left, data, out_file);
    data->variables = current_variables;

    //--------fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
    fwrite(ret, sizeof(char), 1, out_file);
    data->rip += 1

    //write jump
    int jmp_relative_distance = data->rip - jmp_rip;
    long current_file_p = ftell(out_file);
    char number[4];
    WriteLittleInd32(jmp_relative_distance, number);
    fseek(out_file, jmp_file_pointer, 0);
    fwrite(number, sizeof(char), 4, out_file);
    fseek(out_file, current_file_p, 0);
}

void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file) {
    assert(node);
    ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", node->value.var_number * 8);
}

const char var_decl_short[4] = {0x48, 0x89, 0x45, 0x00};
const char var_decl_long[7] = {0x48, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00};

void BytecodeVarDecl(Node* node, BytecodeData* data, FILE* out_file) {
    assert(node);
    ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    // fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", node->value.var_number * 8);
    int var_offset = node->value.var_number * 8;
    if (var_offset <= 127) {
        char short_var_offset = (var_offset & 0xFF);
        fwrite(var_decl_short, sizeof(char), 3, out_file);
        fwrite(&short_var_offset, sizeof(char), 1, out_file);
        data->rip += 4;
    } else {
        fwrite(var_decl_long, sizeof(char), 3, out_file);
        char var_offset_str[4];
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

const char var_get_short[4] = {0x48, 0x8B, 0x45, 0x00};
const char var_get_long[4]  = {0x48, 0x8B, 0x85, 0x00, 0x00, 0x00, 0x00};

void BytecodeName(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    // i need to find var number and mov it to rax
    int var_offset = DAfind(vars, node->value.name) * 8;
    //fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", DAfind(vars, node->value.name) * 8);
    if (var_offset <= 127) {
        char short_var_offset = (var_offset & 0xFF);
        fwrite(var_get_short, sizeof(char), 3, out_file);
        fwrite(&short_var_offset, sizeof(char), 1, out_file);
        data->rip += 4;
    } else {
        fwrite(var_get_short, sizeof(char), 3, out_file);
        char var_offset_str[4];
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

const char add_argument[1] = {0x50};

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

const char check_if_cond_true[4] = {0x48, 0x83, 0xF8, 0x00}
const char jump_to_else[6] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00}

void BytecodeCondition(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    BytecodeParseNode(node->left, data, out_file);
    //fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    fwrite(check_if_cond_true, sizeof(char), 4, out_file);
    data->rip += 4;
    fwrite(jump_to_else, sizeof(char), 6);
    int insert_else_rip = data->rip + 6;
    long insert_else_file_p = ftell(out_file) - 4;
    data->rip += 6;
    BytecodeParseNode(node->right->left, vars, out_file);

    //fprintf(out_file, "jmp LEND%p\nLNOT%p:\n", node, node);
    fwrite(jmp_rel, sizeof(char), 5, out_file);
    data->rip += 5;
    // now insert data->rip as else
    long current_file_p = ftell(out_file);
    long insert_end_file_p = current_file_p - 4;
    int insert_end_rip = data->rip;
    fseek(out_file, insert_else_file_p, 0);
    char rip_str[4];
    WriteLittleInd32(data->rip - insert_else_rip, rip_str);
    fwrite(rip_str, sizeof(char), 4, out_file);
    fseek(out_file, current_file_p, 0);
    BytecodeParseNode(node->right->right, vars, out_file);

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

void BytecodeIfelse(Node* node, DynamicArray* vars, FILE* out_file){
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

const char test_if_rax_is_zero[3] = {0x48, 0x85, 0xC0}
const char jump_if_equal[2] = {0x74, 0x00};

void BytecodeLoop(Node* node, BytecodeData* data, FILE* out_file){
    assert(node);
    //fprintf(out_file, "LOOPSTART%p:\n", node);
    long loop_start_file_p = ftell(out_file);
    int loop_start_rip = data->rip;
    ASMParseNode(node->left, vars, out_file);

    //fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    fwrite(test_if_rax_is_zero, sizeof(char), 3, out_file);

    ASMParseNode(node->right, vars, out_file);
    fprintf(out_file, "LOOPEND%p:\n", node);
}

void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah assg\n");
    ASMParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);
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

void ASMMath(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    //fprintf(out_file, "mov rax, 0 ; here should be something more intellectual\n");
    ASMParseNode(node->left, vars, out_file);
    //fprintf(out_file, "mov rbx, rax\n");
    fprintf(out_file, "push rax\n");
    ASMParseNode(node->right, vars, out_file);
    fprintf(out_file, "pop rbx\n");
    fprintf(out_file, "xchg rax, rbx\nxor rdx, rdx\n");
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

void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("doing constant %d... %p\n", node->value.num, node);
    fprintf(out_file, "mov rax, %d ; var = const %p\n", node->value.num, node);
}
