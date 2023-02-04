STDOUT    equ 1
SYS_write equ 1
SYS_exit  equ 60

extern binsearch

section .text
global _start
_start:
    mov  rdi, lo
    mov  rsi, hi
    mov  rdx, 7
    call binsearch
    cmp  rax, lo+8*3
    jne  failed

    mov  rdi, lo
    mov  rsi, hi
    mov  rdx, 16
    call binsearch
    cmp  rax, lo+8*9
    jne  failed
    
    mov  rdi, lo
    mov  rsi, hi
    mov  rdx, 42
    call binsearch
    cmp  rax, lo+8*18
    jne  failed

    mov  rdi, lo
    mov  rsi, hi
    mov  rdx, 46
    call binsearch
    cmp  rax, lo+8*20
    jne  failed

    mov  rdx, plen
    mov  rsi, pmsg
    jmp  end
failed:
    mov  rdx, flen
    mov  rsi, fmsg
    jmp  end
end:
    mov  rax, SYS_write
    mov  rdi, STDOUT
    syscall
    mov  rax, SYS_exit
    mov  rdi, 0
    syscall

lo   dq 1, 2, 4, 7, 8, 10, 11, 13, 14, 16, 18, 22, 24, 25, 27, 29, 40, 41, 42, 44, 46, 47
hi   dq 51
pmsg db "smoketest passed", 10
plen equ $ - pmsg
fmsg db "smoketest failed", 10
flen equ $ - fmsg
