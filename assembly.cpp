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
xor r9, r9 ; counter
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
push r10
inc r9
jmp InputLoop
; need: r8 = r8 * 10
InputLoopExit:
InputAnsWriteLoop:
cmp r9, 0
je InputAnsWriteLoopExit
mov r10, r8
shl r8, 3
shl r10, 1
add r8, r10
pop r10
add r8, r10
dec r9
jmp InputAnsWriteLoop
InputAnsWriteLoopExit:
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
	ret

section .data
InBuffer: db 0
OutBuffer: times 19 db 0
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
    fprintf(out_file, "cmp rax, 0\njne LNOT%p ; this is condition start", node);
    ParseNode(node->right->left, vars, out_file);
    fprintf(out_file, "LNOT%p:", node);
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
    ParseNode(node->right, vars, out_file);
    fprintf(out_file, "ret\n");
}

void ASMMath(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    fprintf(out_file, "mov rax, 0 ; here should be something more intellectual\n");
}

void ASMConstant(Node* node, DynamicArray* vars, FILE* out_file){
    assert(node);
    fprintf(out_file, "mov rax, %d ; var = const\n", node->value.num);
}
