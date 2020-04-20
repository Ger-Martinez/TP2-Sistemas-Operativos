#include <stdint.h>
#include "MemoryManager.h"
#include <time.h>
#include <screen_driver.h>
#define KILLED 0
#define READY 1
#define BLOCKED 2
#define RUNNING 3  /* no se si hace falta... despues vemos si lo sacamos */
#define READY_TO_BE_USED 4
#define MAX_NUMBER_OF_PROCESSES 30  /* esto se puede ir cambiando */
#define NULL ((void*)0)

uint64_t choose_next_process();
void create_PCB();

typedef struct process_control_block {
    uint8_t state;
    uint64_t stackPointer;
} PCB;

//static void* address_to_store_PBCs = NULL;
static PCB all_blocks[MAX_NUMBER_OF_PROCESSES] = {
{READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, 
{READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, 
{READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, 
{READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, 
{READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, 
{READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, {READY_TO_BE_USED, 0}, 
};
static uint8_t first_call_to_scheduler = 1;

static int borrar_esto = 0;
static int aux = 0;

uint64_t schedule_processes(uint64_t previous_process_SP) {
    timer_handler();  // el handler de Timer Tick del TP de arqui sigue estando
    //drawString("hola");
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
    //drawString("TT handler about to return SP = "); drawNumber(new_process_SP, 0xFFFFFF, 0x000000);
    return new_process_SP;
    //return previous_process_SP;
}

uint64_t choose_next_process(uint64_t previous_process_SP) {
    uint64_t SP_to_return, ready_block_found = 0;
    int ready_block_index = -1;  // borrar esta chanchada cuando sepas programar mejor...
    int running_block_index = -1;
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state != READY_TO_BE_USED) {
            if(/*!ready_block_found && */all_blocks[i].state == READY) {
                drawString("  block num "); drawNumber(i, 0xFFFFFF, 0x000000); drawString(" goes READY-->RUNNING\n");
                ready_block_found = 1;
                all_blocks[i].state = RUNNING;
                SP_to_return = all_blocks[i].stackPointer;
                ready_block_index = i;
            }
            /*if( ready_block_index != i && all_blocks[i]->state == RUNNING)
                running_block_index = i;*/
        }
    }

    if(!ready_block_found){
        //drawString("I am returning previous SP\n");
        return previous_process_SP;   // si no hay un nuevo proceso para correr, elegimos otra vez el mismo.
    } else {
        // aca se podria decir que "si no encontre a nadie, llamo a init" --> PARA DESPUES
        //drawString("I am returning NEW SP\n");
        return SP_to_return;
    }
}

void create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress) {
    int i;
    for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state == READY_TO_BE_USED){  // we find the first empty position
            break;
        }
    }

    if(i < MAX_NUMBER_OF_PROCESSES) {
        all_blocks[i].state = READY;
        all_blocks[i].stackPointer = stackPointerAddress;
    } else {
        // hubo un error
    }
}

void list_all_PBC_array() {
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i].state == READY_TO_BE_USED) {
            drawString("state = READY_TO_BE_USED\n");
        } else {
            drawString("   ");
            drawNumber(all_blocks[i].state, 0xFFFFFF, 0x000000);
            drawString("   ");
            drawNumber(all_blocks[i].stackPointer, 0xFFFFFF, 0x000000);
            drawString("   ");
        }
    }
}