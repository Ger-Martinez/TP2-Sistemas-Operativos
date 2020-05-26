GLOBAL enter_region

section .text

enter_region:
    push rbp
    mov rbp, rsp

    ; hacemos un intercambio entre un registro y una variable de forma ATOMICA
    mov rax, 1
    xchg al, [rdi]  ; xchg 1, lock
    
    mov rsp, rbp
    pop rbp
    ret