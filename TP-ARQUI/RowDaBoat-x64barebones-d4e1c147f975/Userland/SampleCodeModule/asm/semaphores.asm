GLOBAL enter_region
GLOBAL leave_region
EXTERN syscall_block
EXTERN print
EXTERN num_to_string

section .text


; rdi = lock
; rsi = pid
enter_region:
    push rbp
    mov rbp, rsp
    ;push rdi
    push rbx

    ; push rdi
    ; push rcx
    ; mov rcx, [rdi]
    ; mov rdi, rcx
    ; call num_to_string
    ; mov rdi, rax
    ; call print
    ; pop rcx
    ; pop rdi

    ; hacemos un intercambio entre un registro y una variable de forma ATOMICA
    mov rbx, 1
    xchg rbx, [rdi]  ; xchg 1, lock              ; aux <- 1
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
