#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#define NUMBER_OF_COMMANDS 15
#include <stdint.h>

void execute_command(int commands_to_execute[2], uint8_t number_of_commands_to_execute, 
                        char* parameter, uint8_t pid_key, int background);

static char* all_commands[] = 
{
    "mem",
    "kill",
    "test_mm",
    "test_processes",
    "block",
    "ps",
    "loop",
    "nice",
    "inforeg", 
    "help",
    "exception0", 
    "exception6",
    "printmem",
    "showTime",
    "test"
};

#endif