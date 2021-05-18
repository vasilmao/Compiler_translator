input:
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
NextLine: db 0x0a
section .text
global _start
_start:
sub rsp, 8
 ; 48_81 (83 if imm8 instead of imm32)_
mov rbp, rsp
jmp factorial_END
factorial:
mov rbp, rsp
mov rax, [rbp + 0] ; this was name
push rax
mov rax, 1 ; var = const 0x55d7e64a2b50
mov rbx, rax
pop rax
xor rdx, rdx
cmp rax, rbx
setle al
xor rbx, rbx
mov bl, al
xor rax, rax
mov al, bl
cmp rax, 0
je LNOT0x55d7e64a2c40 ; this is condition start
mov rax, 1 ; var = const 0x55d7e64a2be0
ret ; thats it
jmp LEND0x55d7e64a2c40
LNOT0x55d7e64a2c40:
LEND0x55d7e64a2c40:
nop
nop ; condition end
mov rax, [rbp + 0] ; this was name
push rax
push rbp ; save rbp before call
mov rax, [rbp + 0] ; this was name
push rax
mov rax, 1 ; var = const 0x55d7e64a2f50
mov rbx, rax
pop rax
xor rdx, rdx
sub rax, rbx
push rax ; this was argument passing
call factorial ; the call
add rsp, 8
pop rbp ; after call
mov rbx, rax
pop rax
xor rdx, rdx
mul rbx
ret ; thats it
add rbp, 0 ; this was fdecl
ret
factorial_END:
push rbp ; save rbp before call
call input ; the call
add rsp, 0
pop rbp ; after call
mov [rbp + 0], rax ;this was var decl
push rbp ; save rbp before call
push rbp ; save rbp before call
mov rax, [rbp + 0] ; this was name
push rax ; this was argument passing
call factorial ; the call
add rsp, 8
pop rbp ; after call
push rax ; this was argument passing
call print ; the call
add rsp, 8
pop rbp ; after call
add rsp, 8
mov rax, 0x3C
xor rdi, rdi
syscall
