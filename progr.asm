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
sub rsp, 8
 ; 48_81 (83 if imm8 instead of imm32)_
mov rbp, rsp
jmp factorial_END
factorial: ;eeee fdecl
sub rsp, 0
mov rbp, rsp
mov r8, [rbp + 16]
mov rax, r8
push rax
mov rax, 1 ; var = const 0x5626b432a190
mov rbx, rax
pop rax
xor rdx, rdx
cmp rax, rbx
mov rax, 0
setle al
xor rbx, rbx
mov bl, al
xor rax, rax
mov al, bl
cmp rax, 0
je LNOT0x5626b432a280 ; this is condition start
mov rax, 1 ; var = const 0x5626b432a220
add rsp, 0
ret ; thats it
jmp LEND0x5626b432a280
LNOT0x5626b432a280:
LEND0x5626b432a280:
nop
nop ; condition end
mov rax, r8
push rax
mov rax, r8
push rax
mov rax, 1 ; var = const 0x5626b432a590
mov rbx, rax
pop rax
xor rdx, rdx
sub rax, rbx
push rax ; this was argument passing
mov [rbp + 16], r8 ; saving optimized vars
push rbp ; save rbp before call
call factorial ; the call
pop rbp 
add rsp, 8
mov r8, [rbp + 16] ; restoring optimized vars
mov rbx, rax
pop rax
xor rdx, rdx
mul rbx
add rsp, 0
ret ; thats it
add rsp, 0 ; this was fdecl
ret
factorial_END:
mov [rbp + 0], r8 ; saving optimized vars
push rbp ; save rbp before call
call input ; the call
pop rbp 
add rsp, 0
mov r8, [rbp + 0] ; restoring optimized vars
mov r8, rax
mov rax, r8
push rax ; this was argument passing
mov [rbp + 0], r8 ; saving optimized vars
push rbp ; save rbp before call
call factorial ; the call
pop rbp 
add rsp, 8
mov r8, [rbp + 0] ; restoring optimized vars
push rax ; this was argument passing
mov [rbp + 0], r8 ; saving optimized vars
push rbp ; save rbp before call
call print ; the call
pop rbp 
add rsp, 8
mov r8, [rbp + 0] ; restoring optimized vars
add rsp, 8
mov rax, 0x3C
xor rdi, rdi
syscall
