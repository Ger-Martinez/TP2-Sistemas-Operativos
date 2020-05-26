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
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    mov rbx, rsp  ; rbx --> used to backup SP register

    mov rsp, rdi ; now RSP will point to the recently-created stack address

    ;mov rbx, 0x0
    push 0x0    ; push SS=0x0

    push rdi    ; push RSP brought from create_first_process

    ;mov rbx, 0x202
    push 0x202  ; push RFLAGS=0x202

    ;mov rbx, 0x8
    push 0x8    ; push CS=0x8

    push rsi    ; push address of the function brought from create_first_process

    ; we push all the values that the registers will have at the start of the new process
    ; note that the values are increasing, this is just for easy debugging
    ;mov rbx, 0x0
    push 0x0      ; this value will be recovered by RAX once the new process starts
    ;mov rbx, 0x1
    push 0x1      ; this value will be recovered by RBX once the new process starts
    ;mov rbx, 0x2
    push 0x2      ; this value will be recovered by RCX once the new process starts
    ;mov rbx, 0x3
    push 0x3      ; this value will be recovered by RDX once the new process starts
    ;mov rbx, 0x4
    push 0x4      ; this value will be recovered by RBP once the new process starts


    ;mov rbx, rdx  ;the first argument EVERY process is gonna start with: pid_key
    push rdx      ; this value will be recovered by RDI once the new process starts


    ;mov rbx, rcx  ;the second argument EVERY process is gonna start with: where_to_write / where_to_read
    push rcx      ; this value will be recovered by RSI once the new process starts
    
    
    ;mov rbx, 0x7
    push 0x7      ; this value will be recovered by R8 once the new process starts
    ;mov rbx, 0x8
    push 0x8      ; this value will be recovered by R9 once the new process starts
    ;mov rbx, 0x9
    push 0x9      ; this value will be recovered by R10 once the new process starts
    ;mov rbx, 0xA
    push 0xA      ; this value will be recovered by R11 once the new process starts
    ;mov rbx, 0xB
    push 0xB      ; this value will be recovered by R12 once the new process starts
    ;mov rbx, 0xC
    push 0xC      ; this value will be recovered by R13 once the new process starts
    ;mov rbx, 0xD
    push 0xD      ; this value will be recovered by R14 once the new process starts
    ;mov rbx, 0xE
    push 0xE      ; this value will be recovered by R15 once the new process starts


    mov rax, rsp ; this function returns the new RSP address with everything pushed

    mov rsp, rbx ; we make RSP point to THIS program stack-address

    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret
