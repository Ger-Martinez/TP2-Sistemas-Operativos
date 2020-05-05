#include <stdint.h>

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#define DEAD 3
#define READY 4
#define BLOCKED 5
#define BLOCKED_READING 6
// #define RUNNING 3  /* no se si hace falta... despues vemos si lo sacamos */
#define MAX_NUMBER_OF_PROCESSES 30  /* esto se puede ir cambiando */

uint8_t create_process(uint64_t RIP, uint8_t background, uint8_t pid_key);
void exit_process(uint8_t pid_key);
uint64_t kill_process(uint16_t PID);
uint64_t negate_state(uint16_t PID_to_block, uint16_t PID_of_calling_process);

#endif