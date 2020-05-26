// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include "MemoryManager.h"
#include <time.h>
#include <screen_driver.h>
#include "process_manager.h"
#define NULL ((void*)0)
#define INIT_STACK_SIZE 4096
#define MINIMUM_PRIORITY 1

uint64_t choose_next_process();
extern uint64_t configure_stack(uint64_t, void(*foo)(void));
extern void _hlt(void);
static uint8_t configure_init_process();
static void execute_init();

typedef struct process_control_block {
    uint8_t state;
    uint64_t stackPointer;
    uint64_t stackStart;  // will never change. Used for freeing the memory used by the process
    uint32_t PID;
    uint32_t son_of_PID;  // the PID of this process' parent
    uint64_t basePointer; // will never change
    uint8_t priority;    // only changed by the change_priority function
    uint8_t ageing;      // decrements each time the scheduler  is called
} PCB;

// the PCB for the INIT process in keeped outside the list
static PCB init = {READY, 0, 0, 0, 0, 0, 1, 1};

static PCB all_blocks[MAX_NUMBER_OF_PROCESSES];

static uint8_t first_call_to_scheduler = 1;
static uint8_t first_call_to_create_PCB = 1;

static uint32_t pid_number = 1;

static uint8_t current = 1;  // current-1 will always point to the last process chosen by the scheduler
static uint8_t init_was_called = 0;

static uint8_t foreground_process = 0;  // the shell is initially the FG process

uint64_t schedule_processes(uint64_t previous_process_SP) {
    timer_handler();  // el handler de Timer Tick del TP de arqui sigue estando
   
    if(all_blocks[current-1].ageing > 1 && all_blocks[current-1].state == READY){
        // choose the same process
        all_blocks[current-1].ageing --;
        return previous_process_SP;
    }
    all_blocks[current-1].ageing = all_blocks[current-1].priority;

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

    if(previous_process_SP != 0 && !init_was_called){
        all_blocks[current-1].stackPointer = previous_process_SP;
    }

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

uint32_t create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress, uint8_t background, uint8_t parent_pid_key, uint64_t stack_start) {
    // we initialize the array "all_blocks" with all DEAD-processes
    if(first_call_to_create_PCB) {
        for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
            all_blocks[i].state = DEAD;
            all_blocks[i].stackPointer = 0;
            all_blocks[i].stackStart = 0;
            all_blocks[i].basePointer = 0;
            all_blocks[i].PID = 0;
            all_blocks[i].son_of_PID = 0;
            all_blocks[i].priority = 1;
            all_blocks[i].ageing = all_blocks[i].priority;
        }
        // before creating the first process (shell), we create the init process
        uint8_t ret = configure_init_process();
        if(ret == 1) {
            drawString("ERROR in create_PCB_and_insert_it_on_scheduler_queue: could not create init process\n");
            return 1;
        }
    }

    uint8_t i;
    for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state == DEAD){  // we find the first empty position in the PCB's array
            break;
        }
    }

    if(i < MAX_NUMBER_OF_PROCESSES) {
        // we fill the PCB with the required info
        all_blocks[i].state = READY;
        all_blocks[i].stackPointer = stackPointerAddress;
        all_blocks[i].stackStart = stack_start;
        all_blocks[i].basePointer = stackPointerAddress;
        all_blocks[i].PID = pid_number;
        all_blocks[i].priority = 1;
        all_blocks[i].ageing = all_blocks[i].priority;
        pid_number++;

        // the first time we call this function, the SHELL is created, so its parent is init
        if(first_call_to_create_PCB)
            all_blocks[i].son_of_PID = init.PID;
        else
            all_blocks[i].son_of_PID = all_blocks[parent_pid_key].PID;

        first_call_to_create_PCB = 0;

        // if this process was meant to be created in FG, we must block the caller process
        if(background == 0) {
            all_blocks[parent_pid_key].state = BLOCKED;
            foreground_process = i;
            while(1){
                if(all_blocks[parent_pid_key].state == READY)
                    break;
                _hlt();
            }
            // the caller process will only continue when its new FG child executes exit(), which will unblock caller process
        }
        return all_blocks[i].PID;
    } else {
        drawString("ERROR in create_PCB_and_insert_it_on_scheduler_queue: no more PCBs available\n");
        return 1;
    }
}

// init code
static void execute_init() {
    while(1){
        _hlt();
    }
}

static uint8_t configure_init_process(){
    // the address returned by malloc is the END of the stack
    void* init_stack_end = malloc(INIT_STACK_SIZE);
    if(init_stack_end == NULL) {
        drawString("ERROR in configure_init_process: could not malloc stack size\n");
        return 1;
    }

    // the end of the allocked memory is the start of the stack
    void* init_stack_start = (uint64_t*)((uint64_t)init_stack_end + INIT_STACK_SIZE);

    // we create a pointer to the function that init always runs
    void (*init_code)(void);
    init_code = execute_init;
    // push everything in the new stack, preparing the init process for the TT's "pushState" and "iretq"
    uint64_t init_stack_address = configure_stack((uint64_t)init_stack_start, init_code);

    init.stackPointer = init_stack_address;
    init.basePointer = init_stack_address;
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

uint32_t getpid(int pid_key) {
    
    if(pid_key == -1)          // will only be called by the shell
        return pid_number;
    else if(pid_key == -2)     // will only be called by the shell
        return pid_number + 1;
    
    else if(pid_key < -2 || pid_key >= MAX_NUMBER_OF_PROCESSES)
        return 1;  // error
    
    return all_blocks[pid_key].PID;
}

void change_process_state_with_INDEX(uint8_t index, uint8_t state) {
    all_blocks[index].state = state;
    if(state == DEAD) {
        // it means that a process executed an exit()
        free( (void*) (all_blocks[index].stackStart) );
        uint32_t PID_of_parent = all_blocks[index].son_of_PID;
        uint8_t i;
        for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
            if(all_blocks[i].PID == PID_of_parent) {
                if(foreground_process == index) {
                    // our parent process must recover its FG state
                    all_blocks[i].state = READY;
                    foreground_process = i;
                    break;
                }
            }
        }
    }
}

uint64_t change_process_state_with_PID(uint32_t PID, uint8_t state) {
    uint8_t i;
    for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].PID == PID && all_blocks[i].state != DEAD) {
            if(state == DEAD)
                free( (void*) (all_blocks[i].stackStart) );
            all_blocks[i].state = state;
            return 0;
        }
    }
    return 1;  // error
}

uint8_t get_state(uint32_t PID) {
    uint8_t i;
    for(i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].PID == PID && all_blocks[i].state != DEAD) {
            return all_blocks[i].state;
        }
    }
    return 1;  // error
}

uint8_t get_foreground_process() {
    return foreground_process;
}

uint64_t ps(void) {
    drawString("PID        STATE        ¿FG?        PRIORITY        RSP          RBP        CHILD OF\n");
    drawNumber(init.PID, 0xFFFFFF, 0x000000);
    drawString("           READY        NO             1        ");
    drawNumber(init.stackPointer, 0xFFFFFF, 0x000000);
    drawString("        ");
    drawNumber(init.basePointer, 0xFFFFFF, 0x000000);
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
                drawString("YES             ");
            else
                drawString("NO              ");
            drawNumber(all_blocks[i].priority, 0xFFFFFF, 0x000000);
            drawString("        ");
            drawNumber(all_blocks[i].stackPointer, 0xFFFFFF, 0x000000);
            drawString("        ");
            drawNumber(all_blocks[i].basePointer, 0xFFFFFF, 0x000000);
            drawString("        ");
            drawNumber(all_blocks[i].son_of_PID, 0xFFFFFF, 0x000000);
            drawString("\n");
        }
    }
    return 0;
}

uint64_t change_priority(uint8_t pid, uint8_t priority) {
    if(priority < MINIMUM_PRIORITY || pid < 2)
        return 1;  //error
    int i;
    for(i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].PID == pid) {
            all_blocks[i].priority = priority;
            //all_blocks[i].ageing = priority;
            return 0;
        }
    }
    return 1;  // error
}