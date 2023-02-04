; compilation:
;   nasm -felf64 hello.asm
;   ld -o hello hello.o

STDOUT    equ 1
SYS_write equ 1
SYS_exit  equ 60

section .text

len:
    push rbp
    mov  rbp, rsp
    sub  rsp, 8
    mov  rax, rdi
    mov  qword [rbp-8], 0
len_loop:
    cmp  byte [rax], 0
    je   len_end
    inc  rax
    inc  qword [rbp-8]
    jmp  len_loop
len_end:
    mov  rax, qword [rbp-8]
    mov  rsp, rbp
    pop  rbp
    ret

print:
    push rdi
    call len
    pop  rdi
    mov  rdx, rax
    mov  rsi, rdi
    mov  rax, SYS_write
    mov  rdi, STDOUT
    syscall
    ret

global _start
_start:
    pop  rbx
    add  rsp, 8
arg_loop:
    sub  rbx, 1
    jz   end
    mov  rdi, pre
    call print
    pop  rdi
    call print
    mov  rdi, post
    call print
    jmp  arg_loop
end:
    mov  rax, SYS_exit 
    xor  rdi, rdi
    syscall

pre  db "hello, ", 0
post db "!", 10, 0

