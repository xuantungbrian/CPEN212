section .text

global binsearch
binsearch:
	xor rax, rax
	mov r8, rsi
	sub r8, rdi
	SHR r8, 3
	SHL r8, 3
	add r8, rdi
	cmp rdi, rsi
	jg end_bin
	cmp qword [r8], rdx
	je else_if1
	cmp qword [r8], rdx
	jg else_if2
	add rdi, 8
	call binsearch
	ret

end_bin:
	ret
	
else_if1:
	mov rax, r8
	ret

else_if2:
    sub r8, 8
	mov rsi, r8
	call binsearch
	ret
