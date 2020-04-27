#include "stdio.h"
#include "shellCommands.h"
#include "string.h"
#include <stdint.h>
#include "test_mm.h"

char * descriptions[NUMBER_OF_COMMANDS] = 
{ 
    "imprime el estado de la memoria",
    "mata a un proceso segun su PID",
    "TEST: test_mm prueba la Memory Manager", 
    "bloquea otro proceso dado su PID",
    "lista todos los procesos existentes",
    "imprime un saludo cada algunos segundos",
    "Imprime en pantalla el valor actual de todos los registros", 
    "Muestra todos los distintos programas disponibles", 
    "Verifica el funcionamiento de la rutina de excepcion de la division por cero", 
    "Verifica el funcionamiento de la rutina de excepcion de codigo de operacion invalido", 
    "Realiza un volcado de memoria de 32 bytes a partir de la direccion recibida como parametro", 
    "Imprime en pantalla la hora actual"
};



static void inforeg();
static void help();
static void exception0();
static void exception6();
static void printmem(char* parameter);
static void showTime();
static void mem();
static void kill(char* PID);
static void block(char* parameter);
static void testing_mm(uint8_t background, uint8_t pid_key);
static void ps(void);
static void loop(uint8_t background, uint8_t pid_key);

extern uint64_t syscall_read(int, char*, int);
extern uint64_t syscall_create_process(uint64_t, int, uint8_t);
extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);
extern uint64_t syscall_memory_state(void);
extern uint64_t syscall_kill(uint16_t PID);
extern char* num_to_string(int number);
extern uint64_t syscall_block(uint16_t PID);
extern void syscall_ps(void);
extern uint16_t syscall_getpid(uint8_t pid_key);

void execute_command(int command, char* parameter, uint8_t pid_key, int background) {
    switch(command){
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
            block(parameter);
            break;
        }
        case 4:{
            ps();
            break;
        }
        case 5:{
            loop(background, pid_key);
            break;
        }
        case 6:{
            inforeg();
            break;
        }
        case 7:{
            help();
            break;
        }
        case 8:{
            exception0();
            break;
        }
        case 9:{
            exception6();
            break;
        }
        case 10:{
            printmem(parameter);
            break;
        }
        case 11:{
            showTime();
            break;
        }
    }
}

static void ps() {
    syscall_ps();
}

static void testing_mm(uint8_t background, uint8_t pid_key) {
    void (*p)(void);
    p = test_mm;
    // we create a process, which will start its execution on the "test_mm" function
    int ret = syscall_create_process((uint64_t)p, background, pid_key);
    if(ret == 1) {
        print("ERROR: could not create process \"test_mm\"\n");
    }
}


static void block(char* PID) {
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
        uint64_t ret = syscall_block(PID_number);
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
        if(i == 6) {
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