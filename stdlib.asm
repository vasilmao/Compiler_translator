input:
xor r8, r8 ; answer
xor r9, r9 ; counter
InputLoop:
xor rax, rax
mov rdi, 0
mov rsi, InBuffer
mov rdx, 1
syscall
mov r10, byte ptr [InBuffer]
cmp r10, '0'
jl InputLoopExit
cmp r10, '9'
ja InputLoopExit
push r10
inc r9
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
mov rax, qword ptr [rsp + 8]
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
    mov rdi, rcx
    syscall
	ret

section .data
InBuffer: db 0
OutBuffer: times 19 db 0
