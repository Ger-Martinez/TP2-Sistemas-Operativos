#include <stdint.h>
#include "MemoryManager.h"
#include <time.h>
#include <screen_driver.h>
#define DEAD 0
#define READY 1
#define BLOCKED 2
#define RUNNING 3  /* no se si hace falta... despues vemos si lo sacamos */
#define MAX_NUMBER_OF_PROCESSES 30  /* esto se puede ir cambiando */
#define NULL ((void*)0)

uint64_t choose_next_process();
extern uint64_t configure_stack(uint64_t, void(*foo)(void));
extern void _hlt(void);

typedef struct process_control_block {
    uint8_t state;
    uint64_t stackPointer;
    uint16_t PID;
} PCB;

static PCB init = {READY, 0};
#define INIT_STACK_SIZE 64
static uint8_t configure_init_process();

//static void* address_to_store_PBCs = NULL;
static PCB all_blocks[MAX_NUMBER_OF_PROCESSES] = {
{DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, 
{DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, 
{DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}, {DEAD,0,0}
};

static uint8_t first_call_to_scheduler = 1;
static uint8_t first_call_to_create_PCB = 1;

static uint16_t pid_number = 1;

static int borrar_esto = 0;
static int aux = 0;

uint64_t schedule_processes(uint64_t previous_process_SP) {
    timer_handler();  // el handler de Timer Tick del TP de arqui sigue estando
    if((aux=seconds_elapsed()) > borrar_esto) {
        drawNumberInSpecificSpot(20, 20, aux, 0xFFFFFF, 0x000000);
        borrar_esto++;
    }

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
    int ready_block_index = -1;  // borrar esta chanchada cuando sepas programar mejor...
    int running_block_index = -1;
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state != DEAD) {
            if(/*!ready_block_found && */all_blocks[i].state == READY) {
                ready_block_found = 1;
                all_blocks[i].state = RUNNING;
                SP_to_return = all_blocks[i].stackPointer;
                ready_block_index = i;
            }
            /*if( ready_block_index != i && all_blocks[i]->state == RUNNING)
                running_block_index = i;*/
        }
    }

    /*if(!ready_block_found){
        return init.stackPointer;*/
    if(!ready_block_found) {
        return previous_process_SP;
    } else {
        return SP_to_return;
    }
}

uint8_t create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress) {
    if(first_call_to_create_PCB) {
        uint8_t ret = configure_init_process();
        if(ret == 0) {
            drawString("ERROR in create_PCB_and_insert_it_on_scheduler_queue: could not create init process\n");
            return 0;
        }
        first_call_to_create_PCB = 0;
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
        return 1;
    } else {
        drawString("ERROR in create_PCB_and_insert_it_on_scheduler_queue: no more PCBs available\n");
        return 0;
    }
}

static void execute_init() {
    while(1) {
        _hlt();  // sti and hlt instructions
    }
}

static uint8_t configure_init_process() {
    void* init_stack_end = malloc(INIT_STACK_SIZE);
    if(init_stack_end == NULL) {
        drawString("ERROR in configure_init_process: could not malloc stack size\n");
        return 0;
    }
    void* init_stack_start = (uint64_t)init_stack_end + INIT_STACK_SIZE;
    void (*init_code)(void);
    init_code = execute_init;
    uint64_t init_stack_address = configure_stack((uint64_t)init_stack_start, init_code);
    init.stackPointer = init_stack_address;
    init.PID = pid_number;
    pid_number++;
    return 1;
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

/*void list_all_PBC_array() {
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state == DEAD) {
            drawString("state = DEAD\n");
        } else {
            drawString("   ");
            drawNumber(all_blocks[i].state, 0xFFFFFF, 0x000000);
            drawString("   ");
            drawNumber(all_blocks[i].stackPointer, 0xFFFFFF, 0x000000);
            drawString("   ");
        }
    }
}*/