#include "assembly.h"

void IncludeStdlib(FILE* out_file);

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


void ParseNode(Node* node, DynamicArray* vars, FILE* out_file) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case D_TYPE:    ASMFdecl(node, vars, out_file);      break;
        case DECL_TYPE: ASMVarDecl(node, vars, out_file);    break;
        case ID_TYPE:   ASMName(node, vars, out_file);       break;
        case ARG_TYPE:  ASMArgument(node, vars, out_file);   break;
        case COMP_TYPE: ASMComp(node, vars, out_file);       break;
        case STAT_TYPE: ASMStatement(node, vars, out_file);  break;
        case COND_TYPE: ASMCondition(node, vars, out_file);  break;
        case IFEL_TYPE: ASMIfelse(node, vars, out_file);     break;
        case LOOP_TYPE: ASMLoop(node, vars, out_file);       break;
        case ASSG_TYPE: ASMAssg(node, vars, out_file);       break;
        case CALL_TYPE: ASMCall(node, vars, out_file);       break;
        case JUMP_TYPE: ASMReturn(node, vars, out_file);     break;
        case MATH_TYPE: ASMMath(node, vars, out_file);       break;
        case NUMB_TYPE: ASMConstant(node, vars, out_file);   break;
    }
}

void Assembly(Node* root, const char* out_filename) {
    FILE* out_file = NULL;
    open_file(&out_file, out_filename, "w");
    DynamicArray* current_vars = root->value.variables;
    printf("yeah %p\n", current_vars);
    IncludeStdlib(out_file);
    fprintf(out_file, "section .text\nglobal _start\n");
    fprintf(out_file, "_start:\n");
    fprintf(out_file, "sub rsp, %d\n", current_vars->size * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    ParseNode(root, current_vars, out_file);
    fprintf(out_file, "add rsp, %d\nmov rax, 0x3C\nxor rdi, rdi\nsyscall\n", current_vars->size * 8);
    printf("bruh1\n");
    fclose(out_file);
    printf("bruh2\n");
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
    ParseNode(node->right->left, node->value.variables, out_file);
    fprintf(out_file, "ret\n%s_END:\n", fname->value.name);
}

void ASMVarDecl(Node* node, DynamicArray* vars, FILE* out_file) {
    assert(node);
    ParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    fprintf(out_file, "mov [rbp + %d], rax ;this was var decl\n", node->value.var_number * 8);
}

void ASMName(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    // i need to find var number and mov it to rax
    fprintf(out_file, "mov rax, [rbp + %d] ; this was name\n", DAfind(vars, node->value.name) * 8);

}

void ASMArgument(Node* node, DynamicArray* vars, FILE* out_file){
    // need to push arguments, but first argument must be pushed last, so parse right son
    if (node->right != NULL) {
        ParseNode(node->right, vars, out_file);
    }
    ParseNode(node->left, vars, out_file);
    fprintf(out_file, "push rax ; this was argument passing\n");
    assert(node);
}

void ASMComp(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah compound\n");
    ParseNode(node->right, vars, out_file);
}

void ASMStatement(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah statement %p %p\n", node->left, node->right);
    ParseNode(node->left, vars, out_file);
    ParseNode(node->right, vars, out_file);
}

void ASMCondition(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    ParseNode(node->left, vars, out_file);
    fprintf(out_file, "cmp rax, 0\nje LNOT%p ; this is condition start\n", node);
    ParseNode(node->right->left, vars, out_file);
    fprintf(out_file, "LNOT%p:\n", node);
    ParseNode(node->right->right, vars, out_file);
    fprintf(out_file, "nop\nnop ; condition end\n");
}

void ASMIfelse(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    exit(1);
}

void ASMLoop(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    fprintf(out_file, "LOOPSTART%p:\n", node);
    ParseNode(node->left, vars, out_file);
    fprintf(out_file, "cmp rax, 0\nje LOOPEND%p\n", node);
    ParseNode(node->right, vars, out_file);
    fprintf(out_file, "LOOPEND%p:\n", node);
}

void ASMAssg(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah assg\n");
    ParseNode(node->right, vars, out_file);
    // var (left_child) = expression (right_child), result of expression is needed in rax
    fprintf(out_file, "mov [rbp + %d], rax ; var assignation\n", node->value.var_number * 8);
}

void ASMCall(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    printf("yeah call\n");
    fprintf(out_file, "push rbp ; save rbp before call\n");
    ParseNode(node->right, vars, out_file); // arguments in stack
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
    ParseNode(node->right, vars, out_file);
    fprintf(out_file, "ret ; thats it\n");
}

void ASMMath(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    //fprintf(out_file, "mov rax, 0 ; here should be something more intellectual\n");
    ParseNode(node->left, vars, out_file);
    //fprintf(out_file, "mov rbx, rax\n");
    fprintf(out_file, "push rax\n");
    ParseNode(node->right, vars, out_file);
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
