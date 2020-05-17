#include <stdint.h>
#include "stdio.h"
#define MAX_NUMBER_OF_SEMAPHORES 20
#define MAX_NUMBER_OF_PROCESSES_BLOCKED 20

static uint8_t queue_process_in_lock(uint32_t pid, uint8_t index);
static void dequeue_process_in_lock(uint8_t index, uint32_t calling_process_PID);

extern uint64_t syscall_block(uint32_t PID, uint32_t PID_of_calling_process);
extern uint8_t enter_region(uint8_t * lock);
extern char* num_to_string(int num);

typedef struct semaphores {
    uint8_t lock;
    uint8_t value;
    char sem_id;  // needs to be char to store a -1 value
    uint32_t PID_of_process_blocked_by_sem;  // only 1 process can go through LOCK barrier

    // 4 variables for dealing with the queue of processes blocked by LOCK
    uint32_t processes_blocked_by_lock[MAX_NUMBER_OF_PROCESSES_BLOCKED];
    uint8_t number_of_processes_blocked_by_lock;  // queue size
    uint8_t where_to_insert_next_process_blocked_by_lock;
    uint8_t index_of_first_process_blocked_by_lock;
} sem;

static sem semaphores[MAX_NUMBER_OF_SEMAPHORES];
static uint8_t first_call_to_create_semaphore = 1;
static uint8_t number_of_existing_semaphores = 0;

int create_semaphore(uint8_t sem_value, uint8_t sem_unique_id) {
    if(first_call_to_create_semaphore) {
        first_call_to_create_semaphore = 0;
        for(uint8_t i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
            semaphores[i].lock = 0;
            semaphores[i].value = 0;
            semaphores[i].sem_id = -1;
            semaphores[i].PID_of_process_blocked_by_sem = 0;

            semaphores[i].number_of_processes_blocked_by_lock = 0;
            semaphores[i].where_to_insert_next_process_blocked_by_lock = 0;
            semaphores[i].index_of_first_process_blocked_by_lock = 0;

            for(uint8_t j=0; j<MAX_NUMBER_OF_PROCESSES_BLOCKED; j++)
                semaphores[i].processes_blocked_by_lock[j] = 0;
        }
    }

    uint8_t index_of_new_sem, found = 0, i;
    for(i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        // we try to find the first empty semaphore to use
        if( !found && semaphores[i].sem_id == -1 ){
            index_of_new_sem = i;
            found = 1;
        }
        // if the semaphore already exists, it is not an error, and the process connects to it anyways
        if(semaphores[i].sem_id == sem_unique_id)
            return 1;
    }

    if(!found) 
        return -1;
    
    number_of_existing_semaphores++;
    
    semaphores[index_of_new_sem].lock = 0;
    semaphores[index_of_new_sem].value = sem_value;
    semaphores[index_of_new_sem].sem_id = sem_unique_id;
    return 0;
}

static void free_all_queues_and_unblock_all_processes(uint8_t index, uint32_t calling_process_PID) {
    uint32_t pid_to_unblock;
    for(uint8_t i=0; i<MAX_NUMBER_OF_PROCESSES_BLOCKED; i++) {
        pid_to_unblock = semaphores[index].processes_blocked_by_lock[i];
        if(pid_to_unblock != 0) {
            syscall_block(pid_to_unblock, calling_process_PID);
            semaphores[index].processes_blocked_by_lock[i] = 0;
        }
    }

    pid_to_unblock = semaphores[index].PID_of_process_blocked_by_sem;
    if(pid_to_unblock != 0)
        syscall_block(pid_to_unblock, calling_process_PID);

    semaphores[index].number_of_processes_blocked_by_lock = 0;
    semaphores[index].where_to_insert_next_process_blocked_by_lock = 0;
    semaphores[index].index_of_first_process_blocked_by_lock = 0;
    semaphores[index].PID_of_process_blocked_by_sem = 0;
}

uint8_t destroy_semaphore(uint8_t sem_unique_id, uint32_t calling_process_PID) {
    for(uint8_t i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id == sem_unique_id) {

            // The sem_id has to be set to -1 immediately, so that other processes that want to do sem_wait
            // and sem_post return error instantly
            semaphores[i].sem_id = -1;

            number_of_existing_semaphores--;

            // The processes that were left blocked have to be saved, as no process can unblock them now
            // (as no one can make sem_post from now on)
            free_all_queues_and_unblock_all_processes(i, calling_process_PID);
            return 0;
        }
    }
    // if the semaphore was not found, it cannot be destroyed, so we return error
    return 1;
}

static uint8_t valid_sem_id(uint8_t id) {
    uint8_t i;
    for(i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id == id){
            return i;
        }
    }
    return i;
}


// returns 1 if sem does not exists
// returns 2 if no more processes can be blocked for this semaphore
// returns 0 on success
uint8_t sem_wait(uint8_t sem_id, uint32_t pid) {
    uint8_t index_of_sem = valid_sem_id(sem_id), ret;
    if(index_of_sem == MAX_NUMBER_OF_SEMAPHORES)
        return 1;
    
    // if lock != 0, then we block ourselves. If lock == 0, then we pass and now lock=1
    ret = enter_region( &(semaphores[index_of_sem].lock) );

    if(ret == 1) {
        /* POSIBLE ERROR: me estoy bloqueando, pero todavia no aparezco bloqueado. Si ahora el proceso
        adentro de lock sale y quiere buscar a alguien para desbloquear, no me va a encontrar! */
        ret = queue_process_in_lock(pid, index_of_sem);
        if(ret == 1)  // no more space in queue --> return error
            return 2;
        syscall_block(pid, pid);
    }
    
    if(semaphores[index_of_sem].sem_id != -1) {
        if(semaphores[index_of_sem].value == 0){
            /* 
            POSIBLE ERROR: si justo aca corre un proceso que va a hacer sem_post, ese sem_post no va a llegar
            a desbloquear a este proceso justamente porque todavia no se bloqueo! Cuando volvamos a este proceso, 
            se va a bloquear y nadie lo va a salvar 
            */
            semaphores[index_of_sem].PID_of_process_blocked_by_sem = pid;
    	    syscall_block(pid, pid);
        }
        if(semaphores[index_of_sem].sem_id != -1) {
            semaphores[index_of_sem].value --;
            semaphores[index_of_sem].PID_of_process_blocked_by_sem = 0;
            if(semaphores[index_of_sem].number_of_processes_blocked_by_lock != 0)
                dequeue_process_in_lock(index_of_sem, pid);
            else{
                /* POSIBLE ERROR: si justo aca corre otro proceso que intenta entrar a LOCK, se va bloquear porque 
                todavia lock=1. Entonces este proceso nunca se dio cuenta que tenia que salvar al otro proceso ! */
                semaphores[index_of_sem].lock = 0;
            }
            return 0;
        } else {
            print(STD_OUTPUT, "\n Me desbloquie de SEM, pero el semaforo fue destruido. Debo retornar ahora\n");
            return 0;
        }
    } else {
        print(STD_OUTPUT, "\n Me desbloquie de LOCK, pero el semaforo fue destruido. Debo retornar y no volverme a bloquear debido al SEM\n");
        return 0;
    }
}

// sem_post always pierces through the LOCK barrier, as it may need to unblock the only process blocked in SEM
uint8_t sem_post(uint8_t sem_id, uint32_t calling_process_PID) {
    uint32_t pid_to_unblock;
    uint8_t index_of_sem = valid_sem_id(sem_id);
    if(index_of_sem == MAX_NUMBER_OF_SEMAPHORES)
        return 1;
    
    semaphores[index_of_sem].value ++;

    if(semaphores[index_of_sem].value == 1) {
        // it was incremented from 0 to 1 --> must unblock a process
        pid_to_unblock = semaphores[index_of_sem].PID_of_process_blocked_by_sem;
        if(pid_to_unblock != 0) {
            semaphores[index_of_sem].PID_of_process_blocked_by_sem = 0;
            syscall_block(pid_to_unblock, calling_process_PID);
        }
    }
    return 0;
}

static void dequeue_process_in_lock(uint8_t index, uint32_t calling_process_PID) {
    uint8_t aux = semaphores[index].index_of_first_process_blocked_by_lock;
    uint32_t pid_to_unblock = semaphores[index].processes_blocked_by_lock[aux];
    if(semaphores[index].number_of_processes_blocked_by_lock != 0) {
        semaphores[index].processes_blocked_by_lock[aux] = 0;
        semaphores[index].index_of_first_process_blocked_by_lock = 
                        (aux + 1 == MAX_NUMBER_OF_PROCESSES_BLOCKED) ? 0 : aux+1;
        // this syscall will unblock the process with PID = pid_to_unblock
        syscall_block(pid_to_unblock, calling_process_PID);
    }
}

// we always insert new blocked-processes to the right of the last process inserted
static uint8_t queue_process_in_lock(uint32_t pid, uint8_t index) {
    uint8_t i;
    if(semaphores[index].number_of_processes_blocked_by_lock == MAX_NUMBER_OF_PROCESSES_BLOCKED)
        return 1;
    if(semaphores[index].number_of_processes_blocked_by_lock == 0)
        i = 0;
    else
        i = semaphores[index].where_to_insert_next_process_blocked_by_lock;

    semaphores[index].processes_blocked_by_lock[i] = pid;
    semaphores[index].number_of_processes_blocked_by_lock ++ ;
    semaphores[index].where_to_insert_next_process_blocked_by_lock = 
                        (i+1 == MAX_NUMBER_OF_PROCESSES_BLOCKED) ? 0 : i+1;
    return 0;
}

void list_all_semaphores() {
    uint8_t aux;
    if(number_of_existing_semaphores == 0) {
        print(STD_OUTPUT, "There are not semaphores yet. Try running test_synchro\n");
        return;
    }
    print(STD_OUTPUT, "SEM_ID    VALUE    LOCK   PID_OF_PROCESS_BLOCKED_IN_SEM  PID_OF_PROCESSES_BLOCKED_IN_LOCK\n");
    for(uint8_t i = 0; i < MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id != -1) {
            print(STD_OUTPUT, num_to_string(semaphores[i].sem_id) );
            print(STD_OUTPUT, "          ");
            print(STD_OUTPUT, num_to_string(semaphores[i].value) );
            print(STD_OUTPUT, "        ");
            print(STD_OUTPUT, num_to_string(semaphores[i].lock) );
            print(STD_OUTPUT, "               ");
            if(semaphores[i].PID_of_process_blocked_by_sem != 0)
                print(STD_OUTPUT, num_to_string(semaphores[i].PID_of_process_blocked_by_sem) );
            else
                print(STD_OUTPUT, "---");
            print(STD_OUTPUT, "                       ");
            if(semaphores[i].number_of_processes_blocked_by_lock != 0) {
                print(STD_OUTPUT, "See next line \n PID of processes blocked by this semaphore in LOCK:  ");
                for(uint8_t j = 0; j < semaphores[i].number_of_processes_blocked_by_lock; j++) {
                    aux = semaphores[i].processes_blocked_by_lock[j];
                    if(aux !=  0) {
                        print(STD_OUTPUT, num_to_string(aux) );
                        print(STD_OUTPUT, "  ");
                    }
                }
                print(STD_OUTPUT, "\n\n");
            } else {
                print(STD_OUTPUT, "------------\n\n");
            }
            
        }
    }
}