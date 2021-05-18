input:
xor r8, r8 ; answer
xor r9, r9 ; helper
xor rax, rax
mov rdi, 0
mov rsi, InBuffer
mov rdx, 1
syscall

cmp byte [InBuffer], '-'
jne not_minus
push qword 1
jmp after_minus
not_minus:
push qword 0 ; 0 = not minus
jmp deystvie
after_minus:

InputLoop:
xor rax, rax
mov rdi, 0
mov rsi, InBuffer
mov rdx, 1
syscall
deystvie:
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
pop rbx
test rbx, rbx
je input_ret
neg rax
input_ret:
ret

print:
mov rax, qword [rsp + 16]
cmp eax, 0d
jge AfterMinus
push rax
mov rax, 1
mov rdi, 1
mov byte [OutBuffer], '-'
mov rsi, OutBuffer
mov rdx, 1
syscall
pop rax
neg rax
AfterMinus:
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
NextLine: db 0x0a
section .text
global _start
_start:
sub rsp, 16
 ; 48_81 (83 if imm8 instead of imm32)_
mov rbp, rsp
jmp sum_END
sum: ;eeee fdecl
sub rsp, 8
mov rbp, rsp
mov rax, [rbp + 24] ; this was name
push rax
mov rax, [rbp + 32] ; this was name
mov rbx, rax
pop rax
xor rdx, rdx
add rax, rbx
mov [rbp + 0], rax ;this was var decl
mov rax, [rbp + 0] ; this was name
add rsp, 8
ret ; thats it
add rsp, 8 ; this was fdecl
ret
sum_END:
push rbp ; save rbp before call
call input ; the call
pop rbp 
add rsp, 0
mov [rbp + 0], rax ;this was var decl
push rbp ; save rbp before call
call input ; the call
pop rbp 
add rsp, 0
mov [rbp + 8], rax ;this was var decl
mov rax, [rbp + 8] ; this was name
push rax ; this was argument passing
mov rax, [rbp + 0] ; this was name
push rax ; this was argument passing
push rbp ; save rbp before call
call sum ; the call
pop rbp 
add rsp, 16
push rax ; this was argument passing
push rbp ; save rbp before call
call print ; the call
pop rbp 
add rsp, 8
add rsp, 16
mov rax, 0x3C
xor rdi, rdi
syscall
