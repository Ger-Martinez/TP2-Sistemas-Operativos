#ifndef IPC_COMMANDS_H
#define IPC_COMMANDS_H

#include <stdint.h>

void fg(uint8_t pid_key, uint8_t where_to_write);
void wc(uint8_t pid_key, uint8_t where_to_read);
void cat(uint8_t pid_key, uint8_t where_to_read);
void filter(uint8_t pid_key, uint8_t where_to_read);

#endif