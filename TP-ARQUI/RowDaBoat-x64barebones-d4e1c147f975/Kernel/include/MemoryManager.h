#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include <stdint.h>

void* malloc(uint64_t wantedSize);
uint8_t free(void* address);
uint64_t mem_state();

#endif