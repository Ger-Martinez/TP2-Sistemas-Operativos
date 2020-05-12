#ifndef PIPES_H
#define PIPES_H

#include <stdint.h>

uint8_t create_pipe(uint8_t writing_command_ID, uint8_t reading_command_ID);

#endif