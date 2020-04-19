#include "MemoryManager.h"
#include "scheduler.h"
#define FIXED_STACK_SIZE 4096  // 4KB, coincide con la constante en buddy.c

extern uint64_t configure_stack( uint64_t /*void* */ , uint64_t);

void create_process() {
    /*void* stack_end = malloc(FIXED_STACK_SIZE);
    void* stack_start = stack_end + FIXED_STACK_SIZE;*/
    //prepare_stack(stack_start);
}

void create_first_process(uint64_t RIP) {
    void* first_process_stack_end = malloc(FIXED_STACK_SIZE);
    void* first_process_stack_start = (uint64_t)first_process_stack_end + FIXED_STACK_SIZE;

    uint64_t new_stack_address;
    new_stack_address = configure_stack( (uint64_t)first_process_stack_start , RIP );

    create_PCB_and_insert_it_on_scheduler_queue(new_stack_address);
}