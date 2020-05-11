#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>

int create_semaphore(uint8_t sem_value, uint8_t sem_unique_id);
uint8_t destroy_semaphore(uint8_t sem_unique_id);
uint8_t sem_wait(uint8_t sem_id, uint16_t pid);
uint8_t sem_post(uint8_t sem_id, uint16_t calling_process_PID);

#endif