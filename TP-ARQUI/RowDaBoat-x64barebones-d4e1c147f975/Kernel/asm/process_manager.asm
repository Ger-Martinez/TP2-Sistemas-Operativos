GLOBAL configure_stack

configure_stack:
    push rbp
    mov rbp, rsp

    mov rsp, rdi
    push 0x0    ; push SS=0x0
    push rdi    ; push RSP brought from create_first_process
    push 0x202  ; push RFLAGS=0x202
    push 0x8    ; push CS=0x8
    push rsi    ; push RIP brought from create_first_process

    mov rax, rsp ; this function returns the new RSP address with everything pushed

    mov rsp, rbp
    pop rbp
    ret
