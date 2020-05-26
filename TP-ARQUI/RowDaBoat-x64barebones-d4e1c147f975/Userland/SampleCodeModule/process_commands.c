// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include "stdio.h"
#include "string.h"

extern uint64_t syscall_create_process(uint64_t, int, uint8_t, uint8_t);
extern uint64_t syscall_kill(uint32_t PID);
extern char* num_to_string(int number);
extern uint64_t syscall_block(uint32_t PID, uint32_t PID_of_calling_process);
extern void syscall_ps(void);
extern uint32_t syscall_getpid(int pid_key);
extern uint64_t syscall_nice(uint8_t pid, uint8_t priority);
static void receive_nice_parameters(char * pid_array);



void nice(void) {
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
    if(ret == 1)
        print(STD_OUTPUT, "\n ERROR in syscall_nice\n");
}

// a little of busy waiting and hard-coded, just to keep it simple
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

void ps() {
    syscall_ps();
}

void block(char* PID, uint8_t pid_key) {
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

void kill(char* PID) {
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

// loop process
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

// loop command
void loop(uint8_t background, uint8_t pid_key) {
    void (*foo)(uint8_t);
    foo = loop_code;
    // we create the loop process, which will start its execution in the "loop_code"
    int ret = syscall_create_process((uint64_t)foo, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
        print(STD_ERR, "ERROR: could not create process \"loop\"\n");
    }
}