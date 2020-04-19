#include <stdint.h>
#include "MemoryManager.h"
#include <time.h>
#define KILLED 0
#define READY 1
#define BLOCKED 2
#define RUNNING 3  /* no se si hace falta... despues vemos si lo sacamos */
#define MAX_NUMBER_OF_PROCESSES 30  /* esto se puede ir cambiando */
#define NULL ((void*)0)

uint64_t choose_next_process();
void create_PCB();

typedef struct process_control_block {
    uint8_t state;
    uint64_t stackPointer;
} PCB;

static PCB* all_blocks[MAX_NUMBER_OF_PROCESSES] = {NULL};
static uint8_t first_call_to_scheduler = 1;

uint64_t schedule_processes(uint64_t previous_process_SP) {
    timer_handler();  // el handler de TT del TP de arqui sigue estando
    uint64_t new_process_SP;
    if(first_call_to_scheduler) {
        new_process_SP = choose_next_process(0); // en principio no haria falta pasarle el valor 0, ya que la funcion misma de
                                                 // choose_next_process necesita que haya un proceso RUNNING para pasarle el valor del previous_SP
        first_call_to_scheduler = 0;
    } else
        new_process_SP = choose_next_process(previous_process_SP);
    return new_process_SP;
}

uint64_t choose_next_process(uint64_t previous_process_SP) {
    uint64_t SP_to_return, ready_block_found = 0;
    int i_supremo = -1;  // borrar esta chanchada cuando sepas programar mejor...
    for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i] != NULL){
            if(!ready_block_found && all_blocks[i]->state == READY) {
                SP_to_return = all_blocks[i]->stackPointer;
                all_blocks[i]->state == RUNNING;
                ready_block_found = 1;
                i_supremo = i;
            }
            if( i != i_supremo   &&   all_blocks[i]->state == RUNNING ) { 
                /* ESTE if ESTA ASUMIENDO QUE HAY UN SOLO PROCESO CORRIENDO A LA VEZ */
                all_blocks[i]->state == READY;
                all_blocks[i]->stackPointer = previous_process_SP;
            }
        }
    }
    if(!ready_block_found){
        return previous_process_SP;   // si no hay un nuevo proceso para correr, elegimos otra vez el mismo.
    }
    // aca se podria decir que "si no encontre a nadie, llamo a init" --> PARA DESPUES
    return SP_to_return;
}

void create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress) {
    int i;
    for(i=0; i<MAX_NUMBER_OF_PROCESSES; i++) {
        if(all_blocks[i] == NULL)  // we find the first empty position
            break;
    }

    if(i < MAX_NUMBER_OF_PROCESSES) {
        all_blocks[i]->state = READY;
        all_blocks[i]->stackPointer = stackPointerAddress;
    } else {
        // hubo un error
    }
    
}