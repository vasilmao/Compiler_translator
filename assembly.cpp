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

void Assembly(Node* root) {
    FILE* out_file = NULL;
    open_file(&out_file, "prog.asm", "w");
    DynamicArray* current_vars = root->value.variables;
    printf("yeah %p\n", current_vars);
    IncludeStdlib(out_file);
    fprintf(out_file, "section .text\nglobal _start\n");
    fprintf(out_file, "_start:\n");
    fprintf(out_file, "sub rsp, %d\n", current_vars->size * 8);
    fprintf(out_file, "mov rbp, rsp\n");
    ParseNode(root, current_vars, out_file);
    fprintf(out_file, "add rsp, %d\nmov rax, 0x3C\nxor rdi, rdi\nsyscall\n", current_vars->size * 8);
    fclose(out_file);
}

const char* stlib = R"(input:
xor r8, r8 ; answer
xor r9, r9 ; helper
InputLoop:
xor rax, rax
mov rdi, 0
mov rsi, InBuffer
mov rdx, 1
syscall
mov r10b, byte [InBuffer]
cmp r10, '0'
jl InputLoopExit
cmp r10, '9'
ja InputLoopExit
sub r10, '0'
mov r9, r8
shl r8, 3
shl r9, 1
add r8, r9
add r8, r10
;push r10
jmp InputLoop
; need: r8 = r8 * 10
InputLoopExit:
mov rax, r8
ret

print:
mov rax, qword [rsp + 8]
mov rbx, 10
    xor rdx, rdx
    xor rdi, rdi ; counter
	StackNumLoop:
        div rbx ; rax = res, rdx = mod
		push rdx
		inc rdi
		xor rdx, rdx
		cmp rax, 0
		jne StackNumLoop
    xor rcx, rcx
	BuffNumLoop:
		pop rax
        ;call WriteDigitToBuffer
        add rax, '0'
        mov [OutBuffer + rcx], rax
        inc rcx
		dec di
		cmp di, 0
		jne BuffNumLoop
    mov rax, 1
    mov rdi, 1
    mov rsi, OutBuffer
    mov rdx, rcx
    syscall
    mov rax, 1
    mov rdi, 1
    mov rsi, NextLine
    mov rdx, 1
    syscall
	ret

section .data
InBuffer: db 0
OutBuffer: times 19 db 0
NextLine: db 0x20
)";

void IncludeStdlib(FILE* out_file) {
    fprintf(out_file, stlib);
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
