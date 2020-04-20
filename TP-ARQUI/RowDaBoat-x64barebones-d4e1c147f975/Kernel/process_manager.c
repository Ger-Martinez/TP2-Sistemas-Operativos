#include "MemoryManager.h"
#include "scheduler.h"
#include <screen_driver.h>
#define FIXED_STACK_SIZE 4096  // 4KB, coincide con la constante en buddy.c

extern uint64_t configure_stack( uint64_t /*void* */ , uint64_t);

int is_aligned(void *p, int align){
    return ((unsigned long)p & (align - 1)) == 0;
}

void create_first_process(uint64_t RIP) {
    void* first_process_stack_end = malloc(FIXED_STACK_SIZE);
    //drawNumber((uint64_t*)first_process_stack_end, 0xFFFFFF, 0x000000); drawString("\n");

    void* first_process_stack_start = (uint64_t)first_process_stack_end + FIXED_STACK_SIZE;
    //drawNumber((uint64_t*)first_process_stack_start, 0xFFFFFF, 0x000000); drawString("\n");

    uint64_t new_stack_address;
    new_stack_address = configure_stack( (uint64_t)first_process_stack_start , RIP );
    //drawNumber(new_stack_address, 0xFFFFFF, 0x000000); drawString("\n");
    

    create_PCB_and_insert_it_on_scheduler_queue(new_stack_address);
    //list_all_PBC_array();
    //free(first_process_stack_end);
}