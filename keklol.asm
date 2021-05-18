section .text
global _start
_start:
mov rax, r8
mov rax, r9
mov rax, r10
mov rax, r11
mov rax, r12
mov rax, r13
mov rax, r14
mov rax, r15
nop
mov r8, rax
mov r9, rax
mov r10, rax
mov r11, rax
mov r12, rax
mov r13, rax
mov r14, rax
mov r15, rax
nop
mov [rbp+256], r8
mov [rbp+256], r9
mov [rbp+256], r10
mov [rbp+256], r11
mov [rbp+256], r12
mov [rbp+256], r13
mov [rbp+256], r14
mov [rbp+256], r15
nop
mov r8, [rbp+256]
mov r8, [rbp+256]
mov r8, [rbp+256]
mov r9, [rbp+256]
mov r10, [rbp+256]
mov r11, [rbp+256]
mov r12, [rbp+256]
mov r13, [rbp+256]
mov r14, [rbp+256]
mov r15, [rbp+256]
