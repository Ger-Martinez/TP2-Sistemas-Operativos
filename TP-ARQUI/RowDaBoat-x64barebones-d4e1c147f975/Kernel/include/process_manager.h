#include <stdint.h>

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#define DEAD 3
#define READY 4
#define BLOCKED 5
// #define RUNNING 3  /* no se si hace falta... despues vemos si lo sacamos */
#define MAX_NUMBER_OF_PROCESSES 30  /* esto se puede ir cambiando */

uint8_t create_process(uint64_t RIP);
void exit_process(uint8_t pid_key);
uint64_t kill_process(uint16_t PID);
uint64_t negate_state(uint16_t PID);

#endif