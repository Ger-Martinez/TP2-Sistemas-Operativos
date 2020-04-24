#include <stdint.h>
#include "MemoryManager.h"
#include <time.h>
#include <screen_driver.h>
#include "process_manager.h"
#define NULL ((void*)0)
#define INIT_STACK_SIZE 256

uint64_t choose_next_process();
extern uint64_t configure_stack(uint64_t, void(*foo)(void));
extern void _hlt(void);

typedef struct process_control_block {
    uint8_t state;
    uint64_t stackPointer;
    uint16_t PID;
    uint8_t son_of_PID;
} PCB;

static PCB init = {READY, 0, 0, 0};
static uint8_t configure_init_process();
static void execute_init();

//static void* address_to_store_PBCs = NULL;
static PCB all_blocks[MAX_NUMBER_OF_PROCESSES];

static uint8_t first_call_to_scheduler = 1;
static uint8_t first_call_to_create_PCB = 1;

static uint16_t pid_number = 1;

static uint8_t current = 1;
static uint8_t init_was_called = 0;

static uint8_t foreground_process = 0;

uint64_t schedule_processes(uint64_t previous_process_SP) {
    timer_handler();  // el handler de Timer Tick del TP de arqui sigue estando
   
    uint64_t new_process_SP;
    if(first_call_to_scheduler) {
        new_process_SP = choose_next_process(0);
        first_call_to_scheduler = 0;
    } else {
        new_process_SP = choose_next_process(previous_process_SP);
    }
    return new_process_SP;
}

uint64_t choose_next_process(uint64_t previous_process_SP) {
    uint64_t SP_to_return, ready_block_found = 0;
    int i, count;

    if(previous_process_SP != 0 && !init_was_called)
        all_blocks[current-1].stackPointer = previous_process_SP;

    for(i = current, count = 0; !ready_block_found && count != MAX_NUMBER_OF_PROCESSES; i++, count++) {
        if(all_blocks[i].state == READY) {
            current = i + 1;
            if(current == MAX_NUMBER_OF_PROCESSES)
                current = 0;
            SP_to_return = all_blocks[i].stackPointer;
            ready_block_found = 1;
            count--;
        }
        if (i + 1 == MAX_NUMBER_OF_PROCESSES)
            i = -1;
    }

    if (count == MAX_NUMBER_OF_PROCESSES && init_was_called == 0){
        init_was_called = 1;
        return init.stackPointer;
    } else if(count == MAX_NUMBER_OF_PROCESSES && init_was_called == 1) {
        init_was_called = 1;
        init.stackPointer = previous_process_SP;
        return previous_process_SP;
    }

    init_was_called = 0;
    
    return SP_to_return;
}

uint8_t create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress, uint8_t background, uint8_t parent_pid_key) {
    if(first_call_to_create_PCB) {
        for (int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
            all_blocks[i].state = DEAD;
            all_blocks[i].stackPointer = 0;
            all_blocks[i].PID = 0;
            all_blocks[i].son_of_PID = 0;
        }
        uint8_t ret = configure_init_process();
        if(ret == 1) {
            drawString("ERROR in create_PCB_and_insert_it_on_scheduler_queue: could not create init process\n");
            return 1;
        }
    }
    int i;
    for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state == DEAD){  // we find the first empty position
            break;
        }
    }

    if(i < MAX_NUMBER_OF_PROCESSES) {
        all_blocks[i].state = READY;
        all_blocks[i].stackPointer = stackPointerAddress;
        all_blocks[i].PID = pid_number;
        pid_number++;
        if(first_call_to_create_PCB)
            all_blocks[i].son_of_PID = 1;
        else
            all_blocks[i].son_of_PID = all_blocks[parent_pid_key].PID;
        first_call_to_create_PCB = 0;
        if(background == 0) {
            // we must block the process that executed the create_process syscall
            all_blocks[parent_pid_key].state = BLOCKED;
            drawNumber(parent_pid_key, 0xFFFFFF, 0x000000);
            foreground_process = i;
            drawString("-th process will be blocked because it has created a FG process\n");
            _hlt();
        }
        return 0;
    } else {
        drawString("ERROR in create_PCB_and_insert_it_on_scheduler_queue: no more PCBs available\n");
        return 1;
    }
}

static void execute_init() {
    while(1){
        _hlt();
    }
}

static uint8_t configure_init_process(){
    void* init_stack_end = malloc(INIT_STACK_SIZE);
    drawString("start of init = "); drawNumber((uint64_t)init_stack_end, 0xFFFFFF, 0x000000);
    if(init_stack_end == NULL) {
        drawString("ERROR in configure_init_process: could not malloc stack size\n");
        return 1;
    }
    void* init_stack_start = init_stack_end + INIT_STACK_SIZE;
    drawString("  end of init = "); drawNumber((uint64_t)init_stack_start, 0xFFFFFF, 0x000000);
    void (*init_code)(void);
    init_code = execute_init;
    uint64_t init_stack_address = configure_stack((uint64_t)init_stack_start, init_code);
    drawString("  INIT REAL STACK = "); drawNumber(init_stack_address, 0xFFFFFF, 0x000000);
    drawString("\n");
    init.stackPointer = init_stack_address;
    init.PID = pid_number;
    pid_number++;
    return 0;
}

uint8_t get_pid_key() {
    uint8_t i;
    for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++){
        if(all_blocks[i].state == DEAD)
            return i;
    }
    return 0; // just to keep warnings silent
}

uint16_t getpid(uint8_t pid_key) {
    return all_blocks[pid_key].PID;
}

void change_process_state_with_INDEX(uint8_t index, uint8_t state) {
    all_blocks[index].state = state;
    if(state == DEAD) {
        // it means that a process executed an exit()
        uint16_t PID_of_parent = all_blocks[index].son_of_PID;
        uint8_t i;
        for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
            if(all_blocks[i].PID == PID_of_parent) {
                if(foreground_process == index) {
                    all_blocks[i].state = READY;
                    foreground_process = i;
                    break;
                }
                /*if(all_blocks[i].state != BLOCKED && all_blocks[i].state != BLOCKED_READING){
                    all_blocks[i].state = READY;
                    foreground_process = i;
                }
                break;*/
            }
        }
    }
}

uint64_t change_process_state_with_PID(uint16_t PID, uint8_t state) {
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].PID == PID && all_blocks[i].state != DEAD) {
            all_blocks[i].state = state;
            return 0;
        }
    }
    return 1;
}

uint8_t get_state(uint16_t PID) {
    for(int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].PID == PID && all_blocks[i].state != DEAD) {
            return all_blocks[i].state;
        }
    }
    return 1;
}

uint8_t get_foreground_process() {
    return foreground_process;
}

uint64_t ps(void) {
    drawString("PID        STATE        ¿FG?        RSP        CHILD OF\n");
    drawNumber(init.PID, 0xFFFFFF, 0x000000);
    drawString("           READY        NO        ");
    drawNumber(init.stackPointer, 0xFFFFFF, 0x000000);
    drawString("       ----\n");
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state != DEAD) {
            drawNumber(all_blocks[i].PID, 0xFFFFFF, 0x000000);
            drawString("           ");
            switch(all_blocks[i].state){
                case BLOCKED:{
                    drawString("BLOCKED     "); 
                    break;
                }
                case READY:{
                    drawString("READY       ");
                    break;
                }
                case BLOCKED_READING:{
                    drawString("READING     ");
                    break;
                }
                default:{
                    drawString("IMPOSIBLE   ");
                    break;
                }
            }
            if(i == foreground_process)
                drawString("YES        ");
            else
                drawString("NO         ");
            drawNumber(all_blocks[i].stackPointer, 0xFFFFFF, 0x000000);
            drawString("        ");
            drawNumber(all_blocks[i].son_of_PID, 0xFFFFFF, 0x000000);
            drawString("\n");
        }
    }
    return 0;
}