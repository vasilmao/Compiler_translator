input:
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
section .text
global _start
_start:
sub rsp, 16
mov rbp, rsp
push rbp ; save rbp before call
call input ; the call
add rsp, 0
pop rbp ; after call
mov [rbp + 0], rax ;this was var decl
push rbp ; save rbp before call
call input ; the call
add rsp, 0
pop rbp ; after call
mov [rbp + 8], rax ;this was var decl
push rbp ; save rbp before call
mov rax, 0 ; here should be something more intellectual
push rax ; this was argument passing
call print ; the call
add rsp, 8
pop rbp ; after call
add rsp, 16
mov rax, 0x3C
xor rdi, rdi
syscall
