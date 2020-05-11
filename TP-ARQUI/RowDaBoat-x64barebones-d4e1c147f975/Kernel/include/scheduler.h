#ifndef SCHEDULER_H
#define SCHEDULER_H

uint32_t create_PCB_and_insert_it_on_scheduler_queue(uint64_t stackPointerAddress, 
            uint8_t background, uint8_t pid_key, uint64_t stack_start);

uint64_t schedule_processes(uint64_t previous_process_SP);
// no se si aca deberia ir la funcion schedule_process, ya que esta funcion se la llama desde un assembler

uint8_t get_pid_key();
uint32_t getpid(uint8_t pid_key);
void change_process_state_with_INDEX(uint8_t index, uint8_t state);
uint64_t change_process_state_with_PID(uint32_t PID, uint8_t state);
uint8_t get_state(uint32_t PID);

uint8_t get_foreground_process();
uint64_t ps(void);
uint64_t change_priority(uint8_t pid, uint8_t priority);

#endif