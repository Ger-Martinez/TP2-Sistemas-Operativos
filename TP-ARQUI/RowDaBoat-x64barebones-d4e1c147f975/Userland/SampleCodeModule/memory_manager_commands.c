// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdio.h"
#include <stdint.h>

extern uint64_t syscall_memory_state(void);
extern char* num_to_string(int num);

void mem() {
    print(STD_OUTPUT, "  BYTES REMAINING FOR ALLOCATION: ");
    uint64_t free_memory = syscall_memory_state();
    print(STD_OUTPUT,  num_to_string(free_memory) );
}