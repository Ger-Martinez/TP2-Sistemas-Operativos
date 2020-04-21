#ifndef SCHEDULER_H
#define SCHEDULER_H

uint8_t create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress);

uint64_t schedule_processes(uint64_t previous_process_SP);
// no se si aca deberia ir la funcion schedule_process, ya que esta funcion se la llama desde un assembler

//void list_all_PBC_array();  // BORRAR DESPUES
uint8_t get_pid_key();
uint16_t getpid(uint8_t pid_key);


#endif