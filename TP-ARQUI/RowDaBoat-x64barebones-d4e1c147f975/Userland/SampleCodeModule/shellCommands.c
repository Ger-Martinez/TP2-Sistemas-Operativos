#include "stdio.h"
#include "shellCommands.h"
#include "string.h"
#include <stdint.h>
#include "test_mm.h"

char * descriptions[NUMBER_OF_COMMANDS] = 
{ 
"Imprime en pantalla el valor actual de todos los registros", 
"Muestra todos los distintos programas disponibles", 
"Verifica el funcionamiento de la rutina de excepcion de la division por cero", 
"Verifica el funcionamiento de la rutina de excepcion de codigo de operacion invalido", 
"Realiza un volcado de memoria de 32 bytes a partir de la direccion recibida como parametro", 
"Imprime en pantalla la hora actual", 
"hago un test",
"imprime el estado de la memoria",
"mata a un proceso segun su PID",
"TEST: test_mm prueba la Memory Manager", 
"bloquea otro proceso dado su PID"
};

static void inforeg();
static void help();
static void exception0();
static void exception6();
static void printmem(char* parameter);
static void showTime();
static void test(int background, uint8_t pid_key);
static void mem();
static void kill(char* PID);
static void block(char* parameter);

extern uint64_t syscall_read(int, char*, int);
extern uint64_t syscall_create_process(uint64_t, int, uint8_t);
extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);
extern uint64_t syscall_memory_state(void);
extern uint64_t syscall_kill(uint16_t PID);
extern char* num_to_string(int number);
extern uint64_t syscall_block(uint16_t PID);

void execute_command(int command, char* parameter, uint8_t pid_key, int background) {
    switch(command){
        case 0:{
            inforeg();
            break;
        }
        case 1:{
            help();
            break;
        }
        case 2:{
            exception0();
            break;
        }
        case 3:{
            exception6();
            break;
        }
        case 4:{
            printmem(parameter);
            break;
        }
        case 5:{
            showTime();
            break;
        }
        case 6:{
            test(background, pid_key);
            break;
        }
        case 7:{
            mem();
            break;
        }
        case 8:{
            kill(parameter);
            break;
        }
        case 9:{
            test_mm();
            break;
        }
        case 10:{
            block(parameter);
            break;
        }
    }
}

#include "procesoA.h"
#include "procesoB.h"

void ejecutarA(uint8_t pid_key) {
    print("pid_key from PROCESS A = ");
    print(num_to_string(pid_key));
    print_a();
}

void ejecutarB(uint8_t pid_key) {
    print("pid_key from PROCESS B = ");
    print(num_to_string(pid_key));
    print_b(pid_key);
}

static void block(char* PID) {
    if(strcmp(PID, "X") == 0) {
        print("  ERROR: debe elegir a que proceso bloquear");
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
            print("ERROR: no se pudo cambiar el estado del proceso con PID = ");
            print(PID);
        }
    }
}

static void kill(char* PID) {
    if(strcmp(PID, "X") == 0) {
        print("  ERROR: Debe pasarse un PID como parametro");
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
            print("  ERROR: could no kill process with PID = ");
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

static void test(int background, uint8_t pid_key) {
    void (*bb)(uint8_t);
    bb = ejecutarB;
    uint64_t ret = syscall_create_process((uint64_t)bb, background, pid_key);
    if(ret == 1) {
        print("ERROR in syscall create_process");
    }

    //void (*aa)(uint8_t);
    //aa = ejecutarA;
    //syscall_create_process((uint64_t)aa);
    
}

static void inforeg(){
    /* Arqui Legacy */
    print("inforeg");
}

static void help() {
    /* NO OLVIDARSE DE ACTUALIZARLO */
    for(int i=0; i<NUMBER_OF_COMMANDS; i++) {
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
    char time[]= {-1, -1, ':', -1, -1, ':', -1, -1, '\0'};
    syscall_read(3, time, 1);
    print(time);
    print(" UTC");
}