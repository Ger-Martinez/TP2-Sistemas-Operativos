#include "stdio.h"
#include <stdint.h>

extern uint64_t syscall_read(int, char*, int);
extern void syscall_exit(uint8_t pid_key);

static char buffer[10] = {'\0'};
static uint8_t buffer_count = 0;

void print_b(uint8_t pid_key) {
    char resp;
    uint8_t count = 0;
    while( count < 5 ) {
        int ret = syscall_read(0, &resp, 1);
        if(ret != -1) {
            buffer[buffer_count++] = resp;
            count++;
        }
    }
    syscall_exit(pid_key);
}