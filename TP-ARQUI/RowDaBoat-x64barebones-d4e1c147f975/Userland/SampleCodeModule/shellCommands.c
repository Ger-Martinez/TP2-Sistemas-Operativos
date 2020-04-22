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
"TEST: test_mm prueba la Memory Manager"
};

static void inforeg();
static void help();
static void exception0();
static void exception6();
static void printmem(char* parameter);
static void showTime();
static void test();
static void mem();
static void kill(char* PID);

extern uint64_t syscall_read(int, char*, int);
extern uint64_t syscall_write(char*, int);
extern uint64_t syscall_create_process(uint64_t);
extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);
extern uint64_t syscall_memory_state(void);
extern uint64_t syscall_kill(uint16_t PID);
extern char* num_to_string(int number);

void execute_command(int command, char* parameter, uint8_t pid_key) {
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
            test();
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

static void test() {
    /*void* a = syscall_malloc(4096);
    if(a == NULL) {
        print("ERROR in test: could not malloc address\n");
        return;
    }
    char* aa = (char*) a;
    for(int i=0; i<4096; i++) {
        aa[i] = 'c';
    }
    aa[10] = '\0';
    print(aa);
    syscall_free(a);*/
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