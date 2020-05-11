#include <stdint.h>
#include "stdio.h"
#define MAX_NUMBER_OF_SEMAPHORES 20
#define MAX_NUMBER_OF_PROCESSES_LOCKED 20

extern uint64_t syscall_block(uint16_t PID, uint16_t PID_of_calling_process);
static uint8_t queue_process(uint16_t pid, uint8_t index);
static void dequeue_process(uint8_t index, uint16_t pid_of_calling_process);
extern uint8_t leave_region();
extern uint8_t enter_region(uint8_t* lock, uint16_t pid_of_calling_process);
////////////
extern char* num_to_string(int num);
//////////

typedef struct semaphores {
    uint8_t lock;
    uint8_t value;
    char sem_id;  // needs to be char to store a -1 value
    uint16_t pid_blocked_in_sem;  // only one process can be blocked by a semaphore (as only one can go through the LOCK)
    uint16_t pid_queue[MAX_NUMBER_OF_PROCESSES_LOCKED];  // multiple processes can be blocked due to LOCK
    uint8_t queue_size;
    uint8_t where_to_insert_next_process;
    uint8_t index_of_first_process_in_queue;
} sem;

static sem semaphores[MAX_NUMBER_OF_SEMAPHORES];
static uint8_t first_call_to_create_semaphore = 1;


int create_semaphore(uint8_t sem_value, uint8_t sem_unique_id) {
    if(first_call_to_create_semaphore) {
        first_call_to_create_semaphore = 0;
        for(uint8_t i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
            semaphores[i].lock = 0;
            semaphores[i].value = 0;
            semaphores[i].sem_id = -1;
            semaphores[i].pid_blocked_in_sem = 0;
            semaphores[i].where_to_insert_next_process = 0;
            semaphores[i].index_of_first_process_in_queue = 0;
            for(uint8_t j=0; j<MAX_NUMBER_OF_PROCESSES_LOCKED; j++)
                semaphores[i].pid_queue[j] = 0;
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
    
    semaphores[index_of_new_sem].lock = 0;
    semaphores[index_of_new_sem].value = sem_value;
    semaphores[index_of_new_sem].sem_id = sem_unique_id;
    semaphores[index_of_new_sem].queue_size = 0;
    semaphores[index_of_new_sem].index_of_first_process_in_queue = 0;
    semaphores[index_of_new_sem].where_to_insert_next_process = 0;
    return 0;
}

static void free_queue(uint8_t index) {
    for(uint8_t i=0; i<MAX_NUMBER_OF_PROCESSES_LOCKED; i++) {
        semaphores[index].pid_queue[i] = 0;
    }
    semaphores[index].queue_size = 0;
    semaphores[index].where_to_insert_next_process = 0;
    semaphores[index].index_of_first_process_in_queue = 0;
}

uint8_t destroy_semaphore(uint8_t sem_unique_id) {
    for(uint8_t i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id == sem_unique_id) {
            semaphores[i].sem_id = -1;
            /*if(semaphores[i].queue_size != 0)
                free_queue(i);*/
            return 0;
        }
    }
    // if the semaphore was not found, it cannot be destroyed, so we return error
    return 1;
}

static uint8_t valid_sem_id(uint8_t id) {
    int i;
    for(i=0; i<MAX_NUMBER_OF_SEMAPHORES; i++) {
        if(semaphores[i].sem_id == id)
            return i;
    }
    return i;
}


// returns 1 if sem does not exists
// returns 2 if no more processes can be blocked for this semaphore
// returns 0 on success
uint8_t sem_wait(uint8_t sem_id, uint16_t pid) {
    uint8_t index_of_sem = valid_sem_id(sem_id), ret;
    if(index_of_sem == MAX_NUMBER_OF_SEMAPHORES)
        return 1;
    
    // if lock != 0, then we block ourselves. If lock == 0, then we pass and now lock=1
    print(num_to_string(semaphores[index_of_sem].lock )); print("\n");
    //print(num_to_string((uint64_t)&(semaphores[index_of_sem].lock))); print("\n");

    // ret = enter_region( semaphores[index_of_sem].lock , pid);
     ret = enter_region( &(semaphores[index_of_sem].lock) , pid);

    print("\n"); print(num_to_string(semaphores[index_of_sem].lock )); print("\n");
    //print(num_to_string((uint64_t)&(semaphores[index_of_sem].lock))); print("\n");

    if(ret == 1) {
        ret = queue_process(pid, index_of_sem);
        if(ret == 1)
            return 2;
        print("ME BLOQUEO POR LOCK\n");
        syscall_block(pid, pid);
    }
    
    if(semaphores[index_of_sem].value == 0){
        semaphores[index_of_sem].pid_blocked_in_sem = pid;
        /*ret = queue_process(pid, index_of_sem);
        if(ret == 1)  // no more space in queue --> return error
            return 2;*/
        print("ME BLOQUEO POR SEM\n");
    	syscall_block(pid, pid);
    }
    semaphores[index_of_sem].value --;

    semaphores[index_of_sem].lock = 0;
    if(semaphores[index_of_sem].queue_size != 0)
        dequeue_process(index_of_sem, pid);
    // semaphores[index_of_sem].lock = leave_region();
    return 0;
}

uint8_t sem_post(uint8_t sem_id, uint16_t calling_process_PID) {
    uint8_t index_of_sem = valid_sem_id(sem_id);
    if(index_of_sem == MAX_NUMBER_OF_SEMAPHORES)
        return 1;
    semaphores[index_of_sem].value ++;

    uint16_t pid_to_unblock = semaphores[index_of_sem].pid_blocked_in_sem;
    semaphores[index_of_sem].pid_blocked_in_sem = 0;
    syscall_block(pid_to_unblock, calling_process_PID);

    // we unblock a process form the queue
    //dequeue_process(index_of_sem, calling_process_PID);
    // semaphores[index_of_sem].lock = leave_region();
    return 0;
}

static void dequeue_process(uint8_t index, uint16_t calling_process_PID) {
    uint8_t aux = semaphores[index].index_of_first_process_in_queue;
    uint16_t pid_to_unblock = semaphores[index].pid_queue[aux];
    if(semaphores[index].queue_size != 0) {
        semaphores[index].pid_queue[aux] = 0;
        semaphores[index].index_of_first_process_in_queue = (aux + 1 == MAX_NUMBER_OF_PROCESSES_LOCKED) ? 0 : aux+1;
        // this syscall will unblock the process with PID = pid_to_unblock
        syscall_block(pid_to_unblock, calling_process_PID);
    }
}

// we always insert new blocked-processes to the right of the last process inserted
static uint8_t queue_process(uint16_t pid, uint8_t index) {
    uint8_t i;
    if(semaphores[index].queue_size == MAX_NUMBER_OF_PROCESSES_LOCKED)
        return 1;
    else if(semaphores[index].queue_size == 0)
        i = 0;
    else
        i = semaphores[index].where_to_insert_next_process;

    semaphores[index].pid_queue[i] = pid;
    semaphores[index].queue_size ++ ;
    semaphores[index].where_to_insert_next_process = (i+1 == MAX_NUMBER_OF_PROCESSES_LOCKED) ? 0 : i+1;
    return 0;
}