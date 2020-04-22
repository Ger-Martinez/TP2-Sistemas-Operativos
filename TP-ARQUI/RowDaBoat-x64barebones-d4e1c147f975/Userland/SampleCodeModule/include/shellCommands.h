#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#define NUMBER_OF_COMMANDS 8
#include <stdint.h>

void execute_command(int command, char* parameter, uint8_t pid_key);

static char* all_commands[] = 
{"inforeg", 
"help",
"exception0", 
"exception6",
"printmem",
"showTime",
"test",
"mem"
};

#endif