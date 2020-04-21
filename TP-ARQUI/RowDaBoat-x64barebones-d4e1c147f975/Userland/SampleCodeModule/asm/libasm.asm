GLOBAL syscall_write
GLOBAL syscall_read
GLOBAL syscall_create_process
GLOBAL syscall_malloc
GLOBAL syscall_free
GLOBAL syscall_getpid
GLOBAL num_to_string

section .bss
    numstr resb 10  ; used by num_to_string function


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
    mov rcx, 0    ; third argument has no value here
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
    mov rbx, 0    ; second argument has no value here
    mov rcx, 0    ; third argument has no value here
    int 80h
    mov rax, rax  ; syscall has a return value

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret


; --------------------------------
; num_to_string:  convierte un numero a una cadena con los caracteres de ese numero
; argumentos:   RDI = el numero a convertir
; retorno:     eax = cadena del numero
; ---------------------------------
num_to_string:           ;acordate que el cociente de la division lo guarda en eax, el resto en edx, y el divisor en ecx
    push rbp
    mov rbp, rsp
    push rbx
    push rcx
    push rdx

    mov rax, rdi
    mov rcx, 10             ;voy dividiendo de a 10
    mov rdx, 0              ;inicializo en 0 para preparar la division (acordate de EDX:EAX)
    mov rbx, numstr
    add rbx, 9              ;me posiciono al final del string
    mov byte [rbx], 0       ;cargo el 0 final del string
    dec rbx
.sigo:
    div rcx
    add dl, 0x30           ;al sumarle 30 paso un valor numerico a su caracter ASCII
    mov [rbx], dl          ;lleno el espacio vacio con el caracter
    dec rbx
    cmp rax, 0
    je .termino
    mov rdx, 0       ;lleno edx con cero ya que tengo que preparar la division (acordate de EDX:EAX)
    jmp .sigo
.termino:
    inc rbx           ;voy para adelante un espacio ya que ahora mismo estoy parado 1 posicion atras del numero
    mov rax, rbx
    pop rdx
    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret