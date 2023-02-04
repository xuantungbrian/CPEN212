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
	jg end_binsearch
	cmp qword [r8], rdx
	je else_if1
	cmp qword [r8], rdx
	jg else_if2
	add rdi, 8
	jmp binsearch
	
else_if1:
	mov rax, r8
	jmp end_binsearch

else_if2:
	sub r8, 8
	mov rsi, r8
	jmp binsearch

end_binsearch:
	ret
