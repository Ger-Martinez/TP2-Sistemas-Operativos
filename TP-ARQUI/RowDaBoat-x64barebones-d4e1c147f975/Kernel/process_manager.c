#include "MemoryManager.h"
#include "scheduler.h"
#include <screen_driver.h>
#include "process_manager.h"
#include "lib.h"  // for NULL constant
#define FIXED_STACK_SIZE 4096  // 4KB, coincide con la constante en buddy.c
#define PROCESS_EXISTS 1
#define PROCESS_NOT_EXISTS 0

extern uint64_t configure_stack(uint64_t, uint64_t, uint8_t, uint8_t);
extern void _hlt();

// quick way of returning error if no more processes can be created
static uint8_t number_of_free_processes = MAX_NUMBER_OF_PROCESSES;

uint32_t create_process(uint64_t RIP, uint8_t background, uint8_t pid_key, uint8_t input_output_ID) {
    if(number_of_free_processes == 0) {
        return 1;
    }
    else{
        // the address returned by malloc is the END of the stack
        void* process_stack_end = malloc(FIXED_STACK_SIZE);
        if(process_stack_end == NULL) {
            drawString("ERROR in create_process: could not malloc the stack size\n");
            return 1;
        }

        // the end of the allocked memory is the start of the stack
        void* process_stack_start = process_stack_end + FIXED_STACK_SIZE;

        // gets the index in wich this process will have its PCB in the PCB's array
        /* this pid_key will be given to the new process via RDI, so it will start its execution with one parameter.
            This parameter can be used by the process, for example in getpid syscall, as it must retrieve its PID
            using the index of the PCB's array */ 
        uint8_t new_pid_key = get_pid_key();

        // push everything in the new stack, preparing the new process for the TT's "pushState" and "iretq"
        uint64_t new_stack_address;
        new_stack_address = configure_stack( (uint64_t)process_stack_start , RIP , new_pid_key, input_output_ID);

        number_of_free_processes--;
        uint32_t ret = create_PCB_and_insert_it_on_scheduler_queue(new_stack_address, background, pid_key, (uint64_t)process_stack_end);
        if(ret == 1) {
            drawString("ERROR in create_process: could not create PCB\n");
            free(process_stack_end);
            return 1;
        }
        return ret;
    }
}

void exit_process(uint8_t pid_key) {
    number_of_free_processes++;
    change_process_state_with_INDEX(pid_key, DEAD);
    while(1){
        _hlt();
    }
}

uint64_t kill_process(uint32_t PID) {
    number_of_free_processes++;
    return change_process_state_with_PID(PID, DEAD);
}

uint64_t negate_state(uint32_t PID_to_block, uint32_t PID_of_calling_process) {
    uint8_t state = get_state(PID_to_block);
    uint8_t ret;
    if(state == 1)
        return 1;

    if(state == READY)
        ret = change_process_state_with_PID(PID_to_block, BLOCKED);
    else if(state == BLOCKED)
        ret = change_process_state_with_PID(PID_to_block, READY);
    else
        return 1;
    
    if(ret == 1)
        return 1;

    if(PID_to_block == PID_of_calling_process) {
        while(1){
            if(get_state(PID_to_block) == READY)
                break;
            _hlt();
        }
    }
    return 0;
}