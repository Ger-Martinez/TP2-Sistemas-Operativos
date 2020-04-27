GLOBAL configure_stack


;;;;;;;;;;;;;;;;;;;;;;;;;
; arguments:  RDI = address that marks the start of the stack. This function will push a lot of things in this address
;             RSI = address of a function, that will be the first function that the process will execute
;             RDX = pid_key that will be given to the process via RDI
;
; returns:    RAX = address of the stack after is has pushed everything
;                   (it is going to be some bytes before the address received by RDI)
;;;;;;;;;;;;;;;;;;;;;;;;
configure_stack:
    push rbp
    mov rbp, rsp
    push rcx
    push rbx      ; we use RBX to store values
    mov rcx, rsp  ; rcx --> used to backup SP register

    mov rsp, rdi ; now RSP will point to the recently-created stack address

    mov rbx, 0x0
    push rbx    ; push SS=0x0

    push rdi    ; push RSP brought from create_first_process

    mov rbx, 0x202
    push rbx  ; push RFLAGS=0x202

    mov rbx, 0x8
    push rbx    ; push CS=0x8

    push rsi    ; push address of the function brought from create_first_process

    ; we push all the values that the registers will have at the start of the new process
    ; note that the values are increasing, this is just for easy debugging
    mov rbx, 0x0
    push rbx      ; this value will be recovered by RAX once the new process starts
    mov rbx, 0x1
    push rbx      ; this value will be recovered by RBX once the new process starts
    mov rbx, 0x2
    push rbx      ; this value will be recovered by RCX once the new process starts
    mov rbx, 0x3
    push rbx      ; this value will be recovered by RDX once the new process starts
    mov rbx, 0x4
    push rbx      ; this value will be recovered by RBP once the new process starts
    mov rbx, rdx  ;the only argument that EVERY process is gonna start with
    push rbx      ; this value will be recovered by RDI once the new process starts
    mov rbx, 0x6
    push rbx      ; this value will be recovered by RSI once the new process starts
    mov rbx, 0x7
    push rbx      ; this value will be recovered by R8 once the new process starts
    mov rbx, 0x8
    push rbx      ; this value will be recovered by R9 once the new process starts
    mov rbx, 0x9
    push rbx      ; this value will be recovered by R10 once the new process starts
    mov rbx, 0xA
    push rbx      ; this value will be recovered by R11 once the new process starts
    mov rbx, 0xB
    push rbx      ; this value will be recovered by R12 once the new process starts
    mov rbx, 0xC
    push rbx      ; this value will be recovered by R13 once the new process starts
    mov rbx, 0xD
    push rbx      ; this value will be recovered by R14 once the new process starts
    mov rbx, 0xE
    push rbx      ; this value will be recovered by R15 once the new process starts


    mov rax, rsp ; this function returns the new RSP address with everything pushed

    mov rsp, rcx ; we make RSP point to THIS program stack-address
    pop rbx
    pop rcx
    mov rsp, rbp
    pop rbp
    ret
