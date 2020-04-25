GLOBAL syscall_write
GLOBAL syscall_read
GLOBAL syscall_create_process
GLOBAL syscall_malloc
GLOBAL syscall_free
GLOBAL syscall_getpid
GLOBAL syscall_memory_state
GLOBAL syscall_exit
GLOBAL syscall_kill
GLOBAL syscall_block
GLOBAL syscall_ps

section .text

syscall_write:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 4    ; le paso el ID
    mov rbx, 1    ; le paso el File Descriptor a escribir
    mov rcx, rdi  ; le paso el string
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_read:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 3    ; le paso el ID
    mov rbx, rdi  ; le paso "a donde quiero leer"
    mov rcx, rsi  ; le paso la direccion de donde tiene que guardar lo leido
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_create_process:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 2    ; le paso el ID
    mov rbx, rdi  ; le paso la direccion a la que debe apuntar el RIP
    mov rcx, rsi  ; le paso si es de FG o BG
    mov rdx, rdx  ; le paso el pid_key para que este proceso pueda bloquearse, en caso que se cree un proceso en FG
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_malloc:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 45   ; le paso el ID
    mov rbx, rdi  ; le paso el tamaño deseado de memoria
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_free:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 5    ; le paso el ID
    mov rbx, rdi  ; le paso la direccion a ser liberada
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_getpid:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 20    ; le paso el ID
    mov rbx, rdi   ; le paso el pid_key para poder obtener el pid de este proceso
    mov rcx, 0     ; third argument has no value here
    int 80h
    mov rax, rax   ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_exit:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 1    ; le paso el ID
    mov rbx, rdi  ; second argument: pid_key
    mov rcx, 0    ; third argument has no value here
    int 80h

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_kill:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 37    ; le paso el ID
    mov rbx, rdi  ; second argument: PID
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_memory_state:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 6    ; le paso el ID
    mov rbx, 0    ; second argument has no value here
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_block:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 7    ; le paso el ID
    mov rbx, rdi  ; second argument: PID
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

syscall_ps:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx

    mov rax, 8    ; le paso el ID
    mov rbx, 0    ; second argument has no value here
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret