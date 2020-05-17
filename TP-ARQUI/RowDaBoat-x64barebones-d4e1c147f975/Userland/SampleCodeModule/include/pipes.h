#ifndef PIPES_H
#define PIPES_H

#include <stdint.h>

// returns a pipe-ID in the range of [3-33] (not 0, not 1, not 2, so that it doesn't collide with STDIN, STDOUT and STDERR)
char create_pipe(uint32_t writing_process_PID, uint32_t reading_process_PID);
void write_to_pipe(uint8_t pipe_index, char* to_write);
char read_letter_from_pipe(uint8_t pipe_index);
void list_all_pipes();

#endif