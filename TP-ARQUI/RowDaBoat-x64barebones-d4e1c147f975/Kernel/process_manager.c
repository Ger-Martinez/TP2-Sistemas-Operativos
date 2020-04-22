#include "MemoryManager.h"
#include "scheduler.h"
#include <screen_driver.h>
#include "lib.h"  // for NULL constant
#define FIXED_STACK_SIZE 4096  // 4KB, coincide con la constante en buddy.c
#define MAX_NUMBER_OF_PROCESSES 30
#define PROCESS_EXISTS 1
#define PROCESS_NOT_EXISTS 0

extern uint64_t configure_stack(uint64_t, uint64_t, uint8_t);

static uint8_t processes[MAX_NUMBER_OF_PROCESSES] = {PROCESS_NOT_EXISTS};
static uint8_t number_of_free_processes = MAX_NUMBER_OF_PROCESSES;

uint8_t create_process(uint64_t RIP) {
    if(number_of_free_processes == 0) {
        return 0;
    }
    else{
        int i;
        for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
            if(processes[i] != PROCESS_NOT_EXISTS){
                processes[i] = PROCESS_EXISTS;
                number_of_free_processes--;
                break;
            }
        }

        void* process_stack_end = malloc(FIXED_STACK_SIZE);
        if(process_stack_end == NULL) {
            drawString("ERROR in create_process: could not malloc the stack size\n");
            return 0;
        }
        //drawNumber((uint64_t*)process_stack_end, 0xFFFFFF, 0x000000); drawString("\n");

        void* process_stack_start = (uint64_t)process_stack_end + FIXED_STACK_SIZE;
        //drawNumber((uint64_t*)process_stack_start, 0xFFFFFF, 0x000000); drawString("\n");

        uint8_t pid_key = get_pid_key();

        uint64_t new_stack_address;
        new_stack_address = configure_stack( (uint64_t)process_stack_start , RIP , pid_key);
        //drawNumber(new_stack_address, 0xFFFFFF, 0x000000); drawString("\n");
    
        uint8_t ret = create_PCB_and_insert_it_on_scheduler_queue(new_stack_address);
        if(ret == 0) {
            drawString("ERROR in create_process: could not create PCB\n");
            free(process_stack_end);
            return 0;
        }
        return 1;
    }
}