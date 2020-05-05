#include <stdint.h>
#include "stdio.h"
#include "stdio.h"
#define MAX_NUMBER_OF_SEMAPHORES 20

typedef struct pid_queue {
    uint8_t pid;
    struct pid_queue * next;
} pid_queue;

typedef struct semaphores {
    uint8_t lock;
    uint8_t value;
    char sem_id;
    pid_queue * first;
    //uint8_t pid_waiting_for_this_semaphore[MAX_PROCESSES_LINKED_TO_THE_SAME_SEMAPHORE];
} sem;

extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);
extern uint64_t syscall_block(uint16_t PID, uint16_t PID_of_calling_process);
extern char* num_to_string(int num);
static void insert_process_in_queue(uint8_t pid, uint8_t index);

sem semaphores[MAX_NUMBER_OF_SEMAPHORES];
uint8_t first_call_to_create_semaphore = 1;


int create_semaphore(uint8_t sem_value, uint8_t sem_unique_id) {
    if(first_call_to_create_semaphore) {
        first_call_to_create_semaphore = 0;
        for(int i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
            semaphores[i].lock = 0;
            semaphores[i].value = 0;
            semaphores[i].sem_id = -1;
            semaphores[i].first = NULL;
        }
    }

    uint8_t index_of_new_sem, found = 0, i;
    for(i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id == -1 && !found){
            index_of_new_sem = i;
            found = 1;
        }
        if(semaphores[i].sem_id == sem_unique_id)
            return 1;
    }

    if(!found) 
        return -1;
    
    semaphores[index_of_new_sem].lock = 1;
    semaphores[index_of_new_sem].value = sem_value;
    semaphores[index_of_new_sem].sem_id = sem_unique_id;
    semaphores[index_of_new_sem].first = NULL;
    return 0;
}

static void free_queue(uint8_t index) {
    pid_queue* current = semaphores[index].first;
    pid_queue* aux;
    while(current != NULL) {
        aux = current->next;
        syscall_free((void*)current);
        current = aux;
    }
}

uint8_t destroy_semaphore(uint8_t sem_unique_id) {
    for(uint8_t i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id == sem_unique_id) {
            semaphores[i].sem_id = -1;
            if(semaphores[i].first != NULL){
                free_queue(i);
            }
            return 0;
        }
    }
    return 1;
}

static uint8_t valid_sem_id(uint8_t id) {
    int i;
    for(i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++){
        if(semaphores[i].sem_id == id)
            return i;
    }
    return i;
}


// ANOTAR QUE NO TENEMOS CUBIERTO EL CASO DE QUE LA SHELL DESBLOQUEE AL UN PROCESO BLOQUEADO POR UN SEMAFORO
// SIMPLEMENTE VAMOS A DECIR QUE ES "COMPORTAMIENTO INDEFINIDO" (YA QUE SINO ES UN LIO ARREGLARLO)
uint8_t sem_wait(uint8_t sem_id, uint8_t pid) {
    uint8_t index_of_sem = valid_sem_id(sem_id);
    if(index_of_sem == MAX_NUMBER_OF_SEMAPHORES) {
        return 1;
    }
    //block();
    if(semaphores[index_of_sem].value == 0){
        insert_process_in_queue(pid, index_of_sem);
    	syscall_block(pid, pid);
        semaphores[index_of_sem].value --;
    }
    return 0;
    //unblock();
}

uint8_t sem_post(uint8_t sem_id, uint8_t calling_process_PID) {
    uint8_t index_of_sem = valid_sem_id(sem_id);
    if(index_of_sem == MAX_NUMBER_OF_SEMAPHORES) {
        return 1;
    }

    semaphores[index_of_sem].value ++;

    pid_queue * aux = semaphores[index_of_sem].first;
    if(aux != NULL){
        semaphores[index_of_sem].first = aux->next;
        syscall_block(aux->pid, calling_process_PID);
        syscall_free((void*)aux);
    }
    return 0;
}

static void insert_process_in_queue(uint8_t pid, uint8_t index) {
    pid_queue* current = semaphores[index].first;

    void* addr = syscall_malloc(sizeof(struct pid_queue));
    pid_queue* hola;
    hola = addr;
    hola->pid = pid;
    hola->next = NULL;

    if(current == NULL) {
        semaphores[index].first = hola;
    } else {
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = hola;
    }
    return;
}


/*
TODOS SEMAFORO*S:
    |------> S1, S2, S3
              |--------> LOCK
              |--------> VAL
              |--------> PROCESOS BLOCK

    3.07.00
    P1                  P2                         P3
    sem w                  
                        sem w (B)
                        s1 -> procesosB -> p2
                                                    sem w (B)
                                                    s1 -> procesosB -> p2, p3
    sem p
                        s1 -> procesosB -> p3

                        
sem_wait(pid_proceso, sem_id){
    spinlock()
    if(sem == 0)
        void* a = malloc(sizeof(struct pid_queue));
        a[0] = pid;
        for(....){
            if(id = id) {
                //iterando
                semaphores[i].first = a;

            }
        }
        lista de bloqueados.next -> a
        a[0 + sizeof(pid)] = NULL;
    	syscall_de_bloquear_proceso
    else
    	sem--
    	retornar
    spinunlock()
}
*/