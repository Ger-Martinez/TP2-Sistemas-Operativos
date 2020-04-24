#include "MemoryManager.h"
#include "scheduler.h"
#include <screen_driver.h>
#include "process_manager.h"
#include "lib.h"  // for NULL constant
#define FIXED_STACK_SIZE 4096  // 4KB, coincide con la constante en buddy.c
#define MAX_NUMBER_OF_PROCESSES 30
#define PROCESS_EXISTS 1
#define PROCESS_NOT_EXISTS 0

extern uint64_t configure_stack(uint64_t, uint64_t, uint8_t);
extern void _hlt();

static uint8_t number_of_free_processes = MAX_NUMBER_OF_PROCESSES;

uint8_t create_process(uint64_t RIP, uint8_t background, uint8_t pid_key) {
    if(number_of_free_processes == 0) {
        return 1;
    }
    else{

        void* process_stack_end = malloc(FIXED_STACK_SIZE);
        drawString("start of memory = "); drawNumber((uint64_t)process_stack_end, 0xFFFFFF, 0x000000);
        if(process_stack_end == NULL) {
            drawString("ERROR in create_process: could not malloc the stack size\n");
            return 1;
        }

        void* process_stack_start = process_stack_end + FIXED_STACK_SIZE;
        drawString("  end of memory = "); drawNumber((uint64_t)process_stack_start, 0xFFFFFF, 0x000000);

        uint8_t new_pid_key = get_pid_key();
        drawString(" new_pid_key = "); drawNumber(new_pid_key, 0xFFFFFF, 0x000000);

        uint64_t new_stack_address;
        new_stack_address = configure_stack( (uint64_t)process_stack_start , RIP , new_pid_key);
        drawString("  NEW STACK = "); drawNumber(new_stack_address, 0xFFFFFF, 0x000000);
        drawString("\n");
    
        number_of_free_processes--;
        uint8_t ret = create_PCB_and_insert_it_on_scheduler_queue(new_stack_address, background, pid_key);
        if(ret == 1) {
            drawString("ERROR in create_process: could not create PCB\n");
            free(process_stack_end);
            return 1;
        }
        if(background == 0){
            free(process_stack_end);
        }
        drawString("termine de crear un proceso\n");
        return 0;
    }
}

void exit_process(uint8_t pid_key) {
    number_of_free_processes++;
    change_process_state_with_INDEX(pid_key, DEAD);
    _hlt();
}

uint64_t kill_process(uint16_t PID) {
    number_of_free_processes++;
    return change_process_state_with_PID(PID, DEAD);
}

uint64_t negate_state(uint16_t PID) {
    if(get_state(PID) == 1)
        return 1;

    if(get_state(PID) == READY)
        change_process_state_with_PID(PID, BLOCKED);
    else if(get_state(PID) == BLOCKED)
        change_process_state_with_PID(PID, READY);
    return 0;
}