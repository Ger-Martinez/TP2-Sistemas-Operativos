#include "stdio.h"
#include "shellCommands.h"
#include "string.h"
#include <stdint.h>
#include "test_mm.h"
#include "test_processes.h"
#include "semaphores.h"
#include "pipes.h"
/*
#define STD_INPUT 0
#define STD_OUTPUT 1
#define STD_ERR 2
*/
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
    "imprime el stdin tal como lo recibe",
    "Imprime en pantalla el valor actual de todos los registros", 
    "Muestra todos los distintos programas disponibles", 
    "Verifica el funcionamiento de la rutina de excepcion de la division por cero", 
    "Verifica el funcionamiento de la rutina de excepcion de codigo de operacion invalido", 
    "Realiza un volcado de memoria de 32 bytes a partir de la direccion recibida como parametro", 
    "Imprime en pantalla la hora actual",
    "realiza un test"
};

// PROTOTYPES
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
static void ps();
static void loop(uint8_t background, uint8_t pid_key);
static void nice();
static void cat(uint8_t pid_key, uint8_t where_to_read);
static void test(uint8_t pid_key, uint8_t where_to_write);

// EXTERN PROTOTYPES
extern uint64_t syscall_read(int, char*, int);
extern uint64_t syscall_create_process(uint64_t, int, uint8_t, uint8_t);
extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);
extern uint64_t syscall_memory_state(void);
extern uint64_t syscall_kill(uint32_t PID);
extern char* num_to_string(int number);
extern uint64_t syscall_block(uint32_t PID, uint32_t PID_of_calling_process);
extern void syscall_ps(void);
extern uint32_t syscall_getpid(int pid_key);
extern uint64_t syscall_nice(uint8_t pid, uint8_t priority);
extern uint64_t syscall_exit(uint8_t pid_key);

void execute_command(int commands_to_execute[2], uint8_t number_of_commands_to_execute, 
                        char* parameter, uint8_t pid_key, int background) {

    uint8_t where_to_read;
    uint8_t where_to_write;
    uint8_t index_of_commands = 0;
    char ret;

    if(number_of_commands_to_execute > 2){
        print(STD_ERR, "More than 2 commands cannot be executed. Command aborted\n");
        return;
    }
    else if(number_of_commands_to_execute == 2){
        uint32_t pid_1 = syscall_getpid(-1);
        uint32_t pid_2 = syscall_getpid(-2);
        ret = create_pipe(pid_1, pid_2);
        if(ret == -1) {
            print(STD_ERR, "A pipe could not be created. Command aborted\n");
            return;
        }
        where_to_read = ret;
        where_to_write = ret;
    } else {
        where_to_read = STD_INPUT;
        where_to_write = STD_OUTPUT;
    }
    
    // if only one command will be executed, then the functionality is the same as the Arqui project
    while(number_of_commands_to_execute > 0) {
        number_of_commands_to_execute--;
        switch(commands_to_execute[ index_of_commands ]){
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
                cat(pid_key, where_to_read);
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
                test(pid_key, where_to_write);
                break;
            }
        }
        index_of_commands++;
    }
}

static void cat_code(uint8_t pid_key, uint8_t where_to_read) {
    char buffer[128 * 3] = {'\0'};
    uint8_t ret;
    ret = read(where_to_read, &(buffer[0]));
    if(ret == 1) {
        print(STD_ERR, "hubo error en el read\n");
        syscall_exit(pid_key);
    }
    print(STD_OUTPUT, "\n CAT returns:\n");
    print(STD_OUTPUT, buffer);
    syscall_exit(pid_key);
}

static void cat(uint8_t pid_key, uint8_t where_to_read) {
    void (*foo)(uint8_t, uint8_t);
    foo = cat_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 1, pid_key, where_to_read);
    if(ret == 1)
        print(STD_ERR, "ERROR AL CREAR PROCESO\n");
}

static void process_A_code(uint8_t pid_key, uint8_t where_to_write) {
    char buffer[128 * 3] = {'\0'};
    uint8_t k = 0;
    char c;
    while(1){
        c = getChar(STD_INPUT);
        if(c == '_'){  // simula un '\0'
            buffer[k] = '\0';
            print(where_to_write, buffer);
            syscall_exit(pid_key);
        }
        else{
            putchar(STD_OUTPUT, c);
            buffer[k++] = c;
        }
    }
}

static void test(uint8_t pid_key, uint8_t where_to_write) {
    void (*foo)(uint8_t, uint8_t);
    foo  = process_A_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 0, pid_key, where_to_write);
    if(ret == 1)
        print(STD_ERR, "ERROR AL CREAR PROCESS_A\n");
}


static void receive_nice_parameters(char * pid_array) {
    char aux = ' ';
    uint8_t i=0;
    char to_print[2] = {'\0', '\0'};
    while( aux != '\n' && i != 3) {
        aux = getChar(STD_INPUT);
        if(aux != -1 && aux != '\n'){
            *(pid_array + i*sizeof(char)) = aux;
            i++;
            to_print[0] = aux;
            print(STD_OUTPUT, to_print);
        }
    }
}

static void nice(void) {
    print(STD_OUTPUT, "Ingrese el PID del proceso (maximo 3 caracteres): ");
    char pid[4] = {'\0'};          // la 4ta posicion siempre es \0
    char priority[4] = {'\0'};     // la 4ta posicion siempre es \0

    receive_nice_parameters( &(pid[0]) );
    if(strlen(pid) == 0) {
        print(STD_ERR, "\n  ERROR: a PID must be given\n");
        return;
    }
    print(STD_OUTPUT, "\n");
    print(STD_OUTPUT, "Ingrese la nueva prioridad para el proceso (maximo 3 caracteres): ");
    receive_nice_parameters(&(priority[0]));
    if(strlen(priority) == 0) {
        print(STD_ERR, "\n  ERROR: a priority number must be given\n");
        return;
    }
    uint8_t pid_number = string_to_num(pid);
    uint8_t priority_number = string_to_num(priority);
    uint8_t ret = syscall_nice(pid_number, priority_number);
    if(ret == 1) {
        print(STD_ERR, "\n  ERROR: a process with PID = ");
        print(STD_ERR, pid);
        print(STD_ERR, " could not be found");
    }
}

static void ps() {
    syscall_ps();
}

static void testing_mm(uint8_t background, uint8_t pid_key) {
    void (*p)(void);
    p = test_mm;
    // we create a process, which will start its execution on the "test_mm" function
    uint32_t ret = syscall_create_process((uint64_t)p, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
        print(STD_ERR, "ERROR: could not create process \"test_mm\"\n");
    }
}

static void testing_processes(uint8_t background, uint8_t pid_key) {
    void (*p)(uint8_t);
    p = test_processes;
    // we create a process, which will start its execution on the "test_processes" function
    uint32_t ret = syscall_create_process((uint64_t)p, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
        print(STD_ERR, "ERROR: could not create process \"test_processes\"\n");
    }
}

static void block(char* PID, uint8_t pid_key) {
    if(strcmp(PID, "X") == 0) {
        print(STD_ERR, "  ERROR: a PID must be given as an argument.");
        return;
    }
    uint32_t PID_number = string_to_num(PID);
    if(PID_number == 1) {
        print(STD_ERR, "ERROR: cannot block INIT process");
        return;
    } else if(PID_number == 2) {
        print(STD_ERR, "ERROR: cannot block SHELL process");
        return;
    }
    else{
        uint64_t ret = syscall_block(PID_number, syscall_getpid(pid_key));
        if(ret == 1) {
            print(STD_ERR, "ERROR: could not block/unblock process with PID = ");
            print(STD_ERR, PID);
        }
    }
}

static void kill(char* PID) {
    if(strcmp(PID, "X") == 0) {
        print(STD_ERR, "  ERROR: a PID must be given as an argument.");
    } 
    else if(strcmp(PID, "1") == 0) {
        print(STD_ERR, "  ERROR: cannot kill INIT process");
    } 
    else if(strcmp(PID, "2") == 0) {
        print(STD_ERR, "  ERROR: cannot kill SHELL process");
    } 
    else {
        uint32_t PID_number = string_to_num(PID);
        int ret = syscall_kill(PID_number);
        if(ret == 1) {
            print(STD_ERR, "  ERROR: could not kill process with PID = ");
            print(STD_ERR, PID);
            print(STD_ERR, ", because it does NOT EXIST");
        }
    }
}

static void mem() {
    print(STD_OUTPUT, "  BYTES REMAINING FOR ALLOCATION: ");
    uint64_t free_memory = syscall_memory_state();
    print(STD_OUTPUT,  num_to_string(free_memory) );
}

// this is NOT the command loop, this is the code that the loop process will run
// check the next function for the loop-command code
static void loop_code(uint8_t pid_key) {
    uint64_t count = 0;
    uint32_t pid = syscall_getpid(pid_key);
    while(1) {
        count++;
        if(count % 100000000 == 0) {
            count = 1;
            print(STD_OUTPUT, num_to_string( pid ) );
        }
    }
}

// this is the loop command
static void loop(uint8_t background, uint8_t pid_key) {
    void (*foo)(uint8_t);
    foo = loop_code;
    // we create the loop process, which will start its execution in the "loop_code"
    int ret = syscall_create_process((uint64_t)foo, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
        print(STD_ERR, "ERROR: could not create process \"loop\"\n");
    }
}

static void inforeg(){
    /* Arqui Legacy */
    print(STD_OUTPUT, "inforeg");
}

static void help() {
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if(i == 7) {
            print(STD_OUTPUT, "------------------\n  ARQUI LEGACY COMMANDS\n");
        }
        print(STD_OUTPUT, all_commands[i]);
        print(STD_OUTPUT, ":  ");
        print(STD_OUTPUT, descriptions[i]);
        print(STD_OUTPUT, "\n");
    }
}

static void exception0() {
    /* Arqui Legacy */
    print(STD_OUTPUT, "exception0");
}

static void exception6() {
    /* Arqui Legacy */
    print(STD_OUTPUT, "exception6");
}

static void printmem(char* parameter) {
    /* Arqui Legacy */
    print(STD_OUTPUT, "printmem");
}

static void showTime() {
    /* Arqui Legacy */
    char time[]= {-1, -1, ':', -1, -1, ':', -1, -1, '\0'};
    syscall_read(3, time, 1);
    print(STD_OUTPUT, time);
    print(STD_OUTPUT, " UTC");
}