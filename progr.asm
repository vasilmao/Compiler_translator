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
mov rax, 1000000000 ; var = const 0x55f8919e1a70
mov r8, rax
mov rax, 0 ; var = const 0x55f8919e1b50
mov r9, rax
LOOPSTART0x55f8919e2130:
mov rax, r8
push rax
mov rax, 0 ; var = const 0x55f8919e1d10
mov rbx, rax
pop rax
xor rdx, rdx
cmp rax, rbx
mov rax, 0
setg al
xor rbx, rbx
mov bl, al
xor rax, rax
mov al, bl
cmp rax, 0
je LOOPEND0x55f8919e2130
mov rax, r9
push rax
mov rax, 1 ; var = const 0x55f8919e1ec0
mov rbx, rax
pop rax
xor rdx, rdx
add rax, rbx
mov r9, rax
mov rax, r8
push rax
mov rax, 1 ; var = const 0x55f8919e20a0
mov rbx, rax
pop rax
xor rdx, rdx
sub rax, rbx
mov r8, rax
jmp LOOPSTART0x55f8919e2130
LOOPEND0x55f8919e2130:
add rsp, 16
mov rax, 0x3C
xor rdi, rdi
syscall
