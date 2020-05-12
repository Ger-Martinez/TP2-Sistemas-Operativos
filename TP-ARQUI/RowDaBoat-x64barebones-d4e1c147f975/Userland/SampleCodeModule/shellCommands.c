#include "stdio.h"
#include "shellCommands.h"
#include "string.h"
#include <stdint.h>
#include "test_mm.h"
#include "test_processes.h"
#include "semaphores.h"
#include "pipes.h"

char * descriptions[NUMBER_OF_COMMANDS] = 
{ 
    "imprime el estado de la memoria",
    "mata a un proceso segun su PID",
    "TEST: test_mm prueba la Memory Manager",
    "TEST: test_processes prueba el bloqueo, creacion y destruccion de procesos",
    "bloquea otro proceso dado su PID",
    "lista todos los procesos existentes",
    "imprime un saludo cada algunos segundos",
    "cambia la prioridad de un proceso",
    "imprime en pantalla las propiedades de todos los semaforos existentes",
    "Imprime en pantalla el valor actual de todos los registros", 
    "Muestra todos los distintos programas disponibles", 
    "Verifica el funcionamiento de la rutina de excepcion de la division por cero", 
    "Verifica el funcionamiento de la rutina de excepcion de codigo de operacion invalido", 
    "Realiza un volcado de memoria de 32 bytes a partir de la direccion recibida como parametro", 
    "Imprime en pantalla la hora actual",
    "realiza un test"
};

static void inforeg();
static void help();
static void exception0();
static void exception6();
static void printmem(char* parameter);
static void showTime();
static void mem();
static void kill(char* PID);
static void block(char* parameter, uint8_t pid_key);
static void testing_mm(uint8_t background, uint8_t pid_key);
static void testing_processes(uint8_t background, uint8_t pid_key);
static void ps(void);
static void loop(uint8_t background, uint8_t pid_key);
static void nice();
static void cat(uint8_t pid_key);
static void test(uint8_t pid_key);

extern uint64_t syscall_read(int, char*, int);
extern uint64_t syscall_create_process(uint64_t, int, uint8_t);
extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);
extern uint64_t syscall_memory_state(void);
extern uint64_t syscall_kill(uint16_t PID);
extern char* num_to_string(int number);
extern uint64_t syscall_block(uint16_t PID, uint16_t PID_of_calling_process);
extern void syscall_ps(void);
extern uint16_t syscall_getpid(uint8_t pid_key);
extern uint64_t syscall_nice(uint8_t pid, uint8_t priority);
extern uint64_t syscall_exit(uint8_t pid_key);

void execute_command(int commands_to_execute[2], uint8_t number_of_commands_to_execute, 
                        char* parameter, uint8_t pid_key, int background) {

    if(number_of_commands_to_execute > 2)
        print("Impossible\n");
    else if(number_of_commands_to_execute == 2)
        create_pipe(commands_to_execute[0], commands_to_execute[1]);
    
    // if only one command will be executed, then the functionality is the same as the Arqui project
    while(number_of_commands_to_execute > 0) {
        number_of_commands_to_execute--;
        switch(commands_to_execute[0]){
            case 0:{
                mem();
                break;
            }
            case 1:{
                kill(parameter);
                break;
            }
            case 2:{
                testing_mm(background, pid_key);
                break;
            }
            case 3:{
                testing_processes(background, pid_key);
                break;
            }
            case 4:{
                block(parameter, pid_key);
                break;
            }
            case 5:{
                ps();
                break;
            }
            case 6:{
                loop(background, pid_key);
                break;
            }
            case 7:{
                nice();
                break;
            }
            case 8:{
                list_all_semaphores();
                break;
            }
            case 9:{
                cat(pid_key);
                break;
            }
            case 10:{
                inforeg();
                break;
            }
            case 11:{
                help();
                break;
            }
            case 12:{
                exception0();
                break;
            }
            case 13:{
                exception6();
                break;
            }
            case 14:{
                printmem(parameter);
                break;
            }
            case 15:{
                showTime();
                break;
            }
            case 16:{
                test(pid_key);
                break;
            }
        }
    }
}

// NO tiene validaciones de \b como la shell --> escribir cosas faciles
static void cat_code(uint8_t pid_key) {
    char buffer[128] = {'\0'};
    int k = 0;
    char c;
    while(1) {
        c = getChar();
        if(c != -1) {
            if(c == '_') {    // El guion bajo simula un EOF
                buffer[k] = '\0';
                print("\n Cat returns:\n\n");
                print(buffer);
                syscall_exit(pid_key);
            }
            putchar(c);
            buffer[k++] = c;
        } else
            print("Now that we have blocking-READ, this text should never be seen\n");
    }
}

static void cat(uint8_t pid_key) {
    print("Start writing something. When you are ready, type the underscore _\n");
    void (*foo)(uint8_t);
    foo = cat_code;
    // create a process in FOREGROUND
    uint32_t ret = syscall_create_process((uint64_t)foo, 0, pid_key);
    if(ret == 1)
        print("ERROR AL CREAR PROCESO\n");
}

static void process_A_code(uint8_t pid_key) {
    uint16_t pid = syscall_getpid(pid_key);
    int ret = create_semaphore(2, 1);
    if(ret == -1) {
        print("ERROR: NO SE PUDO CREAR EL SEMAFORO\n");
        syscall_exit(pid_key);
    } else if(ret == 1){
        print("PROCESO A SE INTENTO CONECTAR A UN SEMAFORO QUE YA EXISTIA, PERO NO IMPORTA\n");
    }

    ret = sem_wait(1, pid);
    ret = sem_wait(1, pid);
    ret = sem_wait(1, pid);

    ret = destroy_semaphore(1);
    if(ret == 1)
        print("ERROR: PROCESO A NO PUDO DESTRUIR EL SEMAFORO. SEGURO ALGUIEN MAS LO HIZO\n");
    syscall_exit(pid_key);
}

static void process_B_code(uint8_t pid_key) {
    uint16_t pid = syscall_getpid(pid_key);
    int ret = create_semaphore(2, 1);
    if(ret == -1) {
        print("ERROR: NO SE PUDO CREAR EL SEMAFORO\n");
        syscall_exit(pid_key);
    } else if(ret == 1){
        print("PROCESO B SE INTENTO CONECTAR A UN SEMAFORO QUE YA EXISTIA, PERO NO IMPORTA\n");
    }

    sem_post(1, pid);

    ret = destroy_semaphore(1);
    if(ret == 1)
        print("ERROR: PROCESO B NO PUDO DESTRUIR EL SEMAFORO. SEGURO ALGUIEN MAS LO HIZO\n");
    syscall_exit(pid_key);
}

static char buffer[128] = {'\0'};

static void test(uint8_t pid_key) {
    /* void (*foo)(uint8_t);
    void (*foo2)(uint8_t);
    foo  = process_A_code;
    foo2 = process_B_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 1, pid_key);
    if(ret == 1)
        print("ERROR AL CREAR PROCESS_A\n");
    ret = syscall_create_process((uint64_t)foo2, 1, pid_key);
    if(ret == 1)
        print("ERROR AL CREAR PROCESS_B\n"); */
   print("el test no esta haciendo nada ahora\n");
}


static void receive_nice_parameters(char * pid_array) {
    char aux = ' ';
    uint8_t i=0;
    char to_print[2] = {'\0', '\0'};
    while( aux != '\n' && i != 3) {
        aux = getChar();
        if(aux != -1 && aux != '\n'){
            *(pid_array + i*sizeof(char)) = aux;
            i++;
            to_print[0] = aux;
            print(to_print);
        }
    }
}

static void nice(void) {
    print("Ingrese el PID del proceso (maximo 3 caracteres): ");
    char pid[4] = {'\0'};          // la 4ta posicion siempre es \0
    char priority[4] = {'\0'};     // la 4ta posicion siempre es \0

    receive_nice_parameters( &(pid[0]) );
    if(strlen(pid) == 0) {
        print("\n  ERROR: a PID must be given\n");
        return;
    }
    print("\n");
    print("Ingrese la nueva prioridad para el proceso (maximo 3 caracteres): ");
    receive_nice_parameters(&(priority[0]));
    if(strlen(priority) == 0) {
        print("\n  ERROR: a priority number must be given\n");
        return;
    }
    uint8_t pid_number = string_to_num(pid);
    uint8_t priority_number = string_to_num(priority);
    uint8_t ret = syscall_nice(pid_number, priority_number);
    if(ret == 1) {
        print("\n  ERROR: a process with PID = ");
        print(pid);
        print(" could not be found");
    }
}

static void ps() {
    syscall_ps();
}

static void testing_mm(uint8_t background, uint8_t pid_key) {
    void (*p)(void);
    p = test_mm;
    // we create a process, which will start its execution on the "test_mm" function
    uint32_t ret = syscall_create_process((uint64_t)p, background, pid_key);
    if(ret == 1) {
        print("ERROR: could not create process \"test_mm\"\n");
    }
}

static void testing_processes(uint8_t background, uint8_t pid_key) {
    void (*p)(uint8_t);
    p = test_processes;
    // we create a process, which will start its execution on the "test_processes" function
    uint32_t ret = syscall_create_process((uint64_t)p, background, pid_key);
    if(ret == 1) {
        print("ERROR: could not create process \"test_processes\"\n");
    }
}

static void block(char* PID, uint8_t pid_key) {
    if(strcmp(PID, "X") == 0) {
        print("  ERROR: a PID must be given as an argument.");
        return;
    }
    uint16_t PID_number = string_to_num(PID);
    if(PID_number == 1) {
        print("ERROR: cannot block INIT process");
        return;
    } else if(PID_number == 2) {
        print("ERROR: cannot block SHELL process");
        return;
    }
    else{
        uint64_t ret = syscall_block(PID_number, syscall_getpid(pid_key));
        if(ret == 1) {
            print("ERROR: could not block/unblock process with PID = ");
            print(PID);
        }
    }
}

static void kill(char* PID) {
    if(strcmp(PID, "X") == 0) {
        print("  ERROR: a PID must be given as an argument.");
    } 
    else if(strcmp(PID, "1") == 0) {
        print("  ERROR: cannot kill INIT process");
    } 
    else if(strcmp(PID, "2") == 0) {
        print("  ERROR: cannot kill SHELL process");
    } 
    else {
        uint16_t PID_number = string_to_num(PID);
        int ret = syscall_kill(PID_number);
        if(ret == 1) {
            print("  ERROR: could not kill process with PID = ");
            print(PID);
            print(", because it does NOT EXIST");
        }
    }
}

static void mem() {
    print("  BYTES REMAINING FOR ALLOCATION: ");
    uint64_t free_memory = syscall_memory_state();
    print( num_to_string(free_memory) );
}

// this is NOT the command loop, this is the code that the loop process will run
// check the next function for the loop-command code
static void loop_code(uint8_t pid_key) {
    uint64_t count = 0;
    uint16_t pid = syscall_getpid(pid_key);
    while(1) {
        count++;
        if(count % 100000000 == 0) {
            count = 1;
            print( num_to_string( pid ) );
        }
    }
}

// this is the loop command
static void loop(uint8_t background, uint8_t pid_key) {
    void (*foo)(uint8_t);
    foo = loop_code;
    // we create the loop process, which will start its execution in the "loop_code"
    int ret = syscall_create_process((uint64_t)foo, background, pid_key);
    if(ret == 1) {
        print("ERROR: could not create process \"loop\"\n");
    }
}

static void inforeg(){
    /* Arqui Legacy */
    print("inforeg");
}

static void help() {
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if(i == 7) {
            print("------------------\n  ARQUI LEGACY COMMANDS\n");
        }
        print(all_commands[i]);
        print(":  ");
        print(descriptions[i]);
        print("\n");
    }
}

static void exception0() {
    /* Arqui Legacy */
    print("exception0");
}

static void exception6() {
    /* Arqui Legacy */
    print("exception6");
}

static void printmem(char* parameter) {
    /* Arqui Legacy */
    print("printmem");
}

static void showTime() {
    /* Arqui Legacy */
    char time[]= {-1, -1, ':', -1, -1, ':', -1, -1, '\0'};
    syscall_read(3, time, 1);
    print(time);
    print(" UTC");
}