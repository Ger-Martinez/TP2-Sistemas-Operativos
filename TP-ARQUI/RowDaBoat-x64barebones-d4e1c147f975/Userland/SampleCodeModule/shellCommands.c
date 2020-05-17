#include "shellCommands.h"
#include "test_mm.h"
#include "test_processes.h"
#include "test_priority.h"
#include "test_synchro.h"
#include "stdio.h"
#include "IPC_commands.h"
#include "process_commands.h"
#include "memory_manager_commands.h"
#include "test_processes.h"
#include "pipes.h"
#include "semaphores.h"


char* descriptions[NUMBER_OF_COMMANDS] = 
{ 
    "imprime el estado de la memoria",
    "mata a un proceso segun su PID",
    "TEST: test_mm prueba la Memory Manager",
    "TEST: test_processes prueba el bloqueo, creacion y destruccion de procesos",
    "TEST: test_priority prueba las prioridades de los procesos",
    "TEST: test_synchro prueba los mecanismos de sincronizacion",
    "bloquea otro proceso dado su PID",
    "lista todos los procesos existentes",
    "imprime un saludo cada algunos segundos",
    "cambia la prioridad de un proceso",
    "imprime en pantalla las propiedades de todos los semaforos existentes",
    "imprime el stdin tal como lo recibe",
    "cuenta la cantidad de lineas del input",
    "ejecuta un proceso en FG para generar output a un PIPE",
    "filtra las vocales del input",
    "lista todos los pipes existentes",
    "Imprime en pantalla el valor actual de todos los registros", 
    "Muestra todos los distintos programas disponibles", 
    "Verifica el funcionamiento de la rutina de excepcion de la division por cero", 
    "Verifica el funcionamiento de la rutina de excepcion de codigo de operacion invalido", 
    "Realiza un volcado de memoria de 32 bytes a partir de la direccion recibida como parametro", 
    "Imprime en pantalla la hora actual",
};

// PROTOTYPES
static void inforeg();
static void help();
static void exception0();
static void exception6();
static void printmem(char* parameter);
static void showTime();
extern uint32_t syscall_getpid(int pid_key);
extern uint8_t syscall_read(uint8_t where_to_read, char* where_to_store);


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
        // we know a pipe will be executed, so we retrieve the 2 next PIDs that will be generated
        uint32_t pid_1 = syscall_getpid(-1);
        uint32_t pid_2 = syscall_getpid(-2);
        // we store both PIDs in the pipe. This way, the pipe will only follow these process' orders
        ret = create_pipe(pid_1, pid_2);
        if(ret == -1) {
            print(STD_ERR, "A pipe could not be created. Command aborted\n");
            return;
        }
        where_to_read = ret;
        where_to_write = ret;
    } else {
        // if only one command, then the functionality is the same as the Arqui project
        where_to_read = STD_INPUT;
        where_to_write = STD_OUTPUT;
    }
    
    while(number_of_commands_to_execute > 0) {
        number_of_commands_to_execute--;
        switch(commands_to_execute[ index_of_commands ]){
            case 0:
                mem();
                break;
            case 1:
                kill(parameter);
                break;
            case 2:
                testing_mm(background, pid_key);
                break;
            case 3:
                testing_processes(background, pid_key);
                break;
            case 4:
                testing_priority(background, pid_key);
                break;
            case 5:
                ret = test_sync(background, pid_key);     // defined in test_synchro.c
                if(ret != 1)
                    test_no_sync(background, pid_key);  // defined in test_synchro.c
                break;
            case 6:
                block(parameter, pid_key);
                break;
            case 7:
                ps();
                break;
            case 8:
                loop(background, pid_key);
                break;
            case 9:
                nice();
                break;
            case 10:
                list_all_semaphores();
                break;
            case 11:
                cat(pid_key, where_to_read);
                break;
            case 12:
                wc(pid_key, where_to_read);
                break;
            case 13:
                fg(pid_key, where_to_write);
                break;
            case 14:
                filter(pid_key, where_to_read);
                break;
            case 15:
                list_all_pipes();
                break;
            case 16:
                inforeg();
                break;
            case 17:
                help();
                break;
            case 18:
                exception0();
                break;
            case 19:
                exception6();
                break;
            case 20:
                printmem(parameter);
                break;
            case 21:
                showTime();
                break;
        }
        index_of_commands++;
    }
}

static void inforeg(){
    /* Arqui Legacy */
    print(STD_OUTPUT, "inforeg");
}

static void help() {
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if(i == 16) {
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
    syscall_read(3, time);
    print(STD_OUTPUT, time);
    print(STD_OUTPUT, " UTC");
}