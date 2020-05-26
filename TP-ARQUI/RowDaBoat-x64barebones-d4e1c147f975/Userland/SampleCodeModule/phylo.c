// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include "semaphores.h"
#include "stdio.h"
#define MAX_NUMBER_OF_PHYLOS 8
#define MIN_NUMBER_OF_PHYLOS 2
#define NUMBER_OF_PAIRS 2
#define EATING 1
#define NOT_EATING 0

void master_code(uint8_t pid_key);
void print_phylos_state(void);
void phylo_even_code(uint8_t pid_key);
void phylo_odd_code(uint8_t pid_key);
extern uint64_t syscall_create_process(uint64_t, uint8_t, uint8_t);
extern uint64_t syscall_exit(uint8_t pid_key);
extern uint16_t syscall_getpid(uint8_t pid_key);
extern uint64_t syscall_kill(uint32_t PID);
extern char* num_to_string(int num);

typedef struct phylo {
    uint32_t pid_to_connect;
    uint8_t right_sem;
    uint8_t left_sem;
    uint8_t state;   // eating or not
} phylo;

static phylo array[MAX_NUMBER_OF_PHYLOS];
static uint8_t index = 0;
static uint8_t number_of_process_created = 2 * NUMBER_OF_PAIRS;
static uint8_t last_process_added_was_odd = 1;    // TENER CUIDADO CON ESTO, EN EL CASO DE QUE AL PRINCIPIO HAYAN 2-4-6 PHYLOS
                                                    // YA QUE EL ULTIMO AÑADIDO EN ESE CASO NO SERIA ODD, SINO EVEN

void start_phylo(uint8_t pid_key) {

    for(uint8_t i=0; i<MAX_NUMBER_OF_PHYLOS; i++) {
        array[i].pid_to_connect = 0;
        array[i].right_sem = 0;
        array[i].left_sem = 0;
        array[i].state = NOT_EATING;
    }

    int ret;
    void (*master)(uint8_t);
    master = master_code;
    ret = syscall_create_process((uint64_t)master, 0, pid_key);
    if(ret == 1) {
        print(STD_ERR, "ERROR: could not create phylo process\n");
        return;
    }
}

void master_code(uint8_t pid_key) {
    int ret;
    uint8_t sem_ID_generator = 1;
    uint8_t last_sem_id_generated = 0;
    uint32_t master_PID = syscall_getpid(pid_key);

    void (*phylo_even)(uint8_t);
    void (*phylo_odd)(uint8_t);
    phylo_even = phylo_even_code;
    phylo_odd = phylo_odd_code;

    // we create 2 pairs of PHYLOS
    for(uint8_t i=0; i < NUMBER_OF_PAIRS; i++) {

        ret = syscall_create_process((uint64_t)phylo_even, 1, pid_key);
        if(ret == 1) {
            print(STD_ERR, "ERROR: could not create phylo_even process\n");
            syscall_exit(pid_key);
        } else {
            array[index].pid_to_connect = ret;
            ret = create_semaphore(1, sem_ID_generator);
            if(ret == -1 || ret == 1) {
                print(STD_ERR, "ERROR: could not create semaphore\n");
                syscall_exit(pid_key);
            }
            if(last_sem_id_generated != 0) {
                array[index].left_sem = last_sem_id_generated;
            }
            array[index++].right_sem = sem_ID_generator++;
        }

        ret = syscall_create_process((uint64_t)phylo_odd, 1, pid_key);
        if(ret == 1) {
            print(STD_ERR, "ERROR: could not create phylo_odd process\n");
            syscall_exit(pid_key);
        } else {
            array[index].pid_to_connect = ret;
            ret = create_semaphore(1, sem_ID_generator);
            if(ret == -1 || ret == 1) {
                print(STD_ERR, "ERROR: could not create semaphore\n");
                syscall_exit(pid_key);
            }
            if(i == NUMBER_OF_PAIRS - 1) {
                array[0].left_sem = sem_ID_generator;
            }
            array[index].left_sem = sem_ID_generator - 1;
            last_sem_id_generated = sem_ID_generator;
            array[index++].right_sem = sem_ID_generator++;
        }
    }

    print(STD_OUTPUT, "You can add more phylos to the table, up to a MAXIMUM of 8, with the letter \"a\"\n");
    print(STD_OUTPUT, "You can also remove phylos from the table, to a MINIMUM of 2 phylos, with the letter \"r\"\n");
    print(STD_OUTPUT, "When all phylos have finished, please kill this master process with the letter \"k\"\n");
    
    char c;
    while(1) {
        c = getChar(STD_INPUT);
        if(c != -1) {
            if(c == 'k'){  // kill this process
                print(STD_OUTPUT, "You have chosen to kill master process\n");
                for(uint8_t i=0; i<MAX_NUMBER_OF_PHYLOS; i++){
                    destroy_semaphore(array[i].right_sem, master_PID);
                }
                syscall_exit(pid_key);
            }
            else if(c == 'a'){   // add a phylo
                if(number_of_process_created != MAX_NUMBER_OF_PHYLOS) {

                    void (*aux)(uint8_t);
                    if(last_process_added_was_odd){
                        aux = phylo_even_code;
                        last_process_added_was_odd = 0;
                    } else {
                        aux = phylo_odd_code;
                        last_process_added_was_odd = 1;
                    }

                    ret = syscall_create_process((uint64_t)aux, 1, pid_key);
                    if(ret == 1) 
                        print(STD_ERR, "ERROR: could not create a new phylo process in the table\n");
                    else {
                        array[index].pid_to_connect = ret;
                        ret = create_semaphore(1, sem_ID_generator);
                        if(ret == 1)
                            print(STD_ERR, "ERROR: could not create a new FORK (aka semaphore) in the table\n");
                        else {
                            array[0].left_sem = sem_ID_generator;
                            array[index].left_sem = last_sem_id_generated++;
                            array[index++].right_sem = sem_ID_generator++;
                            number_of_process_created++;
                        }
                    }
                } else {
                    print(STD_ERR, "You can't add more phylos!\n");
                }
            }
            else if(c == 'r') {   // kill a phylo
                if(number_of_process_created > MIN_NUMBER_OF_PHYLOS) {
                    number_of_process_created--;
                    last_process_added_was_odd = (last_process_added_was_odd == 1) ? 0 : 1;
                    index -= 1;
                    array[0].left_sem = array[index].left_sem;

                    uint8_t sem_to_destroy = array[index].right_sem;
                    ret = destroy_semaphore(sem_to_destroy, master_PID);
                    if(ret == 1)
                        print(STD_ERR, "Could not destroy FORK (aka semaphore) in the table\n");

                    uint32_t last_phylo_PID = array[index].pid_to_connect;
                    syscall_kill(last_phylo_PID);

                    array[index].pid_to_connect = 0;
                    array[index].left_sem = 0;
                    array[index].right_sem = 0;
                    array[index].state = NOT_EATING;
                }
                else
                    print(STD_ERR, "You cant't kill any more phylos!");
            }
            else if(c == ' ')
                print_phylos_state();
        }
    }
}

// EVEN PHYLO EATS FIRST FROM RIGHT
// ODD  PHYLO EATS FIRST FROM LEFT

void phylo_even_code(uint8_t pid_key) {
    int ret;
    uint32_t this_process_PID = syscall_getpid(pid_key);
    uint8_t i;

    for(i=0; i<MAX_NUMBER_OF_PHYLOS; i++)
        if(array[i].pid_to_connect == this_process_PID)
            break;
    
    uint8_t left_sem;
    uint8_t right_sem;
    if(i != MAX_NUMBER_OF_PHYLOS) {
        left_sem = array[i].left_sem;
        right_sem = array[i].right_sem;
    } else {
        print(STD_ERR, "Impossible!\n");
        syscall_exit(pid_key);
        return;  // never reached, just to keep warnings quiet
    }
    

    ret = sem_wait(right_sem, this_process_PID);
    if(ret == 1 || ret == 2) {
        print(STD_ERR, "Phylo_even could not execute SEM_WAIT\n");
        syscall_exit(pid_key);
    }
    
    ret = sem_wait(left_sem, this_process_PID);
    if(ret == 1 || ret == 2) {
        print(STD_ERR, "Phylo_even could not execute SEM_WAIT\n");
        syscall_exit(pid_key);
    }

    array[i].state = EATING;
    for(uint64_t i=0; i<100000; i++) {
        for(uint64_t j=0; j<10000; j++) {
            // THE PHYLO EATS
        }
    }
    array[i].state = NOT_EATING;

    print(STD_OUTPUT, "Phylo with PID = ");
    print(STD_OUTPUT, num_to_string(this_process_PID));
    print(STD_OUTPUT, " have finished eating. It will now give away its forks and die\n");

    ret = sem_post(left_sem, this_process_PID);
    if(ret == 1) {
        print(STD_ERR, "Phylo_even could not execute SEM_POST\n");
        syscall_exit(pid_key);
    }

    ret = sem_post(right_sem, this_process_PID);
    if(ret == 1) {
        print(STD_ERR, "Phylo_even could not execute SEM_POST\n");
        syscall_exit(pid_key);
    }

    syscall_exit(pid_key);
}

void phylo_odd_code(uint8_t pid_key) {
    int ret;
    uint32_t this_process_PID = syscall_getpid(pid_key);
    uint8_t i;

    for(i=0; i<MAX_NUMBER_OF_PHYLOS; i++)
        if(array[i].pid_to_connect == this_process_PID)
            break;
    
    uint8_t left_sem;
    uint8_t right_sem;
    if(i != MAX_NUMBER_OF_PHYLOS) {
        left_sem = array[i].left_sem;
        right_sem = array[i].right_sem;
    } else {
        print(STD_ERR, "Impossible!\n");
        syscall_exit(pid_key);
        return;  // never reached, just to keep warnings quiet
    }

    ret = sem_wait(left_sem, this_process_PID);
    if(ret == 1 || ret == 2) {
        print(STD_ERR, "Phylo_odd could not execute SEM_WAIT\n");
        syscall_exit(pid_key);
    }
    
    ret = sem_wait(right_sem, this_process_PID);
    if(ret == 1 || ret == 2) {
        print(STD_ERR, "Phylo_odd could not execute SEM_WAIT\n");
        syscall_exit(pid_key);
    }

    array[i].state = EATING;
    for(uint64_t i=0; i<100000; i++) {
        for(uint64_t j=0; j<10000; j++) {
            // THE PHYLO EATS
        }
    }
    array[i].state = NOT_EATING;

    print(STD_OUTPUT, "Phylo with PID = ");
    print(STD_OUTPUT, num_to_string(this_process_PID));
    print(STD_OUTPUT, " have finished eating. It will now give away its forks and die\n");

    ret = sem_post(right_sem, this_process_PID);
    if(ret == 1) {
        print(STD_ERR, "Phylo_odd could not execute SEM_POST\n");
        syscall_exit(pid_key);
    }

    ret = sem_post(left_sem, this_process_PID);
    if(ret == 1) {
        print(STD_ERR, "Phylo_odd could not execute SEM_POST\n");
        syscall_exit(pid_key);
    }

    syscall_exit(pid_key);
}

void print_phylos_state(void) {
    for(uint8_t i=0; i<MAX_NUMBER_OF_PHYLOS; i++) {
        if(array[i].pid_to_connect != 0) {
            if(array[i].state == EATING) {
                print(STD_OUTPUT, "E  ");
            } else {
                print(STD_OUTPUT, "--  ");
            }
        }
    }
    print(STD_OUTPUT, "\n");
}