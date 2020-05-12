GLOBAL enter_region
GLOBAL leave_region
EXTERN syscall_block
EXTERN print
EXTERN num_to_string

section .text

enter_region:
    push rbp
    mov rbp, rsp
    ;push rdi
    push rbx

    ; hacemos un intercambio entre un registro y una variable de forma ATOMICA
    mov rbx, 1
    xchg bl, [rdi]  ; xchg 1, lock              ; aux <- rbx
                                                 ; rbx <- [rdi]    FUNCIONA MAL
                                                 ; [rdi] <- aux    FUNCIONA BIEN
    
    cmp rbx, 0  ; was lock zero?
    je .not_block
.block:
    mov rax, 1
    jmp .fin
.not_block:
    mov rax, 0
.fin:
    pop rbx
    ;pop rdi
    mov rsp, rbp
    pop rbp
    ret



leave_region:
    push rbp
    mov rbp, rsp

    mov rax, 0  ; mov lock, 0

    mov rsp, rbp
    pop rbp
    ret
