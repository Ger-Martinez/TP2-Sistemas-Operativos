GLOBAL configure_stack

configure_stack:
    push rbp
    mov rbp, rsp
    push rcx
    push rbx
    mov rcx, rsp  ; rcx --> used to backup SP register

    mov rsp, rdi ; pass the recently-created stack address to the SP

    mov rbx, 0x0
    push rbx    ; push SS=0x0

    push rdi    ; push RSP brought from create_first_process

    mov rbx, 0x202
    push rbx  ; push RFLAGS=0x202

    mov rbx, 0x8
    push rbx    ; push CS=0x8

    push rsi    ; push RIP brought from create_first_process

    mov rbx, 0x0
    push rbx      ; this value will be recovered by RAX
    mov rbx, 0x1
    push rbx      ; this value will be recovered by RBX
    mov rbx, 0x2
    push rbx      ; this value will be recovered by RCX
    mov rbx, 0x3
    push rbx      ; this value will be recovered by RDX
    mov rbx, 0x4
    push rbx      ; this value will be recovered by RBP
    mov rbx, rdx  ;the only argument that EVERY process is gonna start with
    push rbx      ; this value will be recovered by RDI
    mov rbx, 0x6
    push rbx      ; this value will be recovered by RSI
    mov rbx, 0x7
    push rbx      ; this value will be recovered by R8
    mov rbx, 0x8
    push rbx      ; this value will be recovered by R9
    mov rbx, 0x9
    push rbx      ; this value will be recovered by R10
    mov rbx, 0xA
    push rbx      ; this value will be recovered by R11
    mov rbx, 0xB
    push rbx      ; this value will be recovered by R12
    mov rbx, 0xC
    push rbx      ; this value will be recovered by R13
    mov rbx, 0xD
    push rbx      ; this value will be recovered by R14
    mov rbx, 0xE
    push rbx      ; this value will be recovered by R15


    mov rax, rsp ; this function returns the new RSP address with everything pushed

    mov rsp, rcx ; we return to THIS program stack address
    pop rbx
    pop rcx
    mov rsp, rbp
    pop rbp
    ret
