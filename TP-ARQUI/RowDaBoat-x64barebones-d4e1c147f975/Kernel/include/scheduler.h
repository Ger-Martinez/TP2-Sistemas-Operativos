#ifndef SCHEDULER_H
#define SCHEDULER_H

void create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress);

uint64_t schedule_processes(uint64_t previous_process_SP);
// no se si aca deberia ir la funcion schedule_process, ya que esta funcion se la llama desde un assembler


#endif