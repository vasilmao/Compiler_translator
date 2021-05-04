section .text
global _start
_start:
mov rbp, rax
mov rbp, rsp
mov rax, rbp
mov rax, 0
mov rax, 1
mov rax, 128
mov rax, 1024
mov rax, 1000000000
mov [rbp], rax
mov [rbp + 0], rax
mov [rbp + 8], rax
mov rax, [rbp]
mov rax, [rbp + 0]
mov rax, [rbp + 8]
mov rax, [rbp + 16]
mov rax, [rbp + 24]
mov rax, [rbp + 32]
mov rax, [rbp + 64]
mov rbx, [rbp]
mov rbx, [rbp + 0]
mov rbx, [rbp + 8]
mov rbx, [rbp + 16]
mov rbx, [rbp + 24]
mov rbx, [rbp + 32]
mov rbx, [rbp + 64]
mov r8, [rbp]
mov r8, [rbp + 0]
mov r8, [rbp + 8]
mov r8, [rbp + 16]
mov r8, [rbp + 24]
mov r8, [rbp + 32]
mov r8, [rbp + 64]
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
push rbp
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
pop rax
push rbp
pop rbp
