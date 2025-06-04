    .text
    .intel_syntax noprefix
    .globl main
main:
    push rbp
    mov rbp, rsp
    mov dword ptr [rbp - 4], 23
    pop rbp
    ret

    .globl b
b:
    push rbp
    mov rbp, rsp
    pop rbp
    ret

