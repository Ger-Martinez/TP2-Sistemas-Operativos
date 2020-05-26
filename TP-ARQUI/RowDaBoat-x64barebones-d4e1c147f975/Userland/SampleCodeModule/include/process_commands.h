#ifndef PROCESS_COMMANDS_H
#define PROCESS_COMMANDS_H

#include <stdint.h>

void nice(void);
void ps();
void block(char* PID, uint8_t pid_key);
void kill(char* PID);
void loop(uint8_t background, uint8_t pid_key);

#endif