#include <stdint.h>
#define MAX_NUMBER_OF_PIPES 5

typedef struct pipe{
    uint8_t pipe_unique_ID;
    char process_that_write; // use char to store a -1 value
    char process_that_read;  // use char to store a -1 value
} pipe;

static pipe all_pipes[MAX_NUMBER_OF_PIPES];
static uint8_t number_of_existing_pipes = 0;

static uint8_t first_time_creating_pipe = 1;
static uint8_t ID_generator = 1;  // will increment over time


uint8_t create_pipe(uint8_t writing_command_ID, uint8_t reading_command_ID) {
    uint8_t i;
    if(first_time_creating_pipe) {
        first_time_creating_pipe = 0;
        for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
            all_pipes[i].pipe_unique_ID = 0;
            all_pipes[i].process_that_write = -1;
            all_pipes[i].process_that_read = -1;
        }
    }
    for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
        if(all_pipes[i].pipe_unique_ID == 0) {
            all_pipes[i].process_that_write = writing_command_ID;
            all_pipes[i].process_that_read = reading_command_ID;
            return 0;
        }
    }
    return 1;
    
}

uint8_t destroy_pipe(uint8_t pipe_ID) {
    if(number_of_existing_pipes == 0)
        return 1;

    uint8_t i;
    for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
        if(all_pipes[i].pipe_unique_ID == pipe_ID) {
            all_pipes[i].pipe_unique_ID = 0;
            return 0;
        }
    }
    return 1;
}