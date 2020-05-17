#include <stdint.h>
#include "semaphores.h"
#include "stdio.h"
#define MAX_NUMBER_OF_PIPES 5
#define BUFFER_SIZE (48*128) / 4 

extern char* num_to_string(int num);

typedef struct pipe{
    uint8_t pipe_unique_ID;
    uint8_t semaphore_ID;
    uint8_t buffer[BUFFER_SIZE];  // un cuarto de pantalla
    uint16_t buffer_writing_index;
    uint16_t buffer_reading_index;
    uint32_t writing_process_pid;
    uint32_t reading_process_pid;
} pipe;

static pipe all_pipes[MAX_NUMBER_OF_PIPES];
static uint8_t number_of_existing_pipes = 0;

static uint8_t first_time_creating_pipe = 1;
static uint8_t ID_generator = 1;  // will increment over time

char create_pipe(uint32_t writing_process_PID, uint32_t reading_process_PID) {
    uint8_t i, ret;
    uint16_t j;
    if(first_time_creating_pipe) {
        first_time_creating_pipe = 0;
        for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
            all_pipes[i].pipe_unique_ID = 0;
            all_pipes[i].semaphore_ID = 0;
            all_pipes[i].buffer_writing_index = 0;
            all_pipes[i].buffer_reading_index = 0;
            all_pipes[i].writing_process_pid = 0;
            all_pipes[i].reading_process_pid = 0;
            for(j=0; j<BUFFER_SIZE; j++){
                all_pipes[i].buffer[j] = 0;
            }
        }
    }
    for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
        if(all_pipes[i].pipe_unique_ID == 0) {
            all_pipes[i].pipe_unique_ID = ID_generator;
            all_pipes[i].semaphore_ID = ID_generator;
            ID_generator++;

            ret = create_semaphore(0, all_pipes[i].semaphore_ID);
            if(ret == -1 || ret == 1) {
                print(STD_ERR, "ERROR: Semaphore could not be created for this pipe\n");
                return -1;
            }
            all_pipes[i].writing_process_pid = writing_process_PID;
            all_pipes[i].reading_process_pid = reading_process_PID;
            all_pipes[i].buffer_writing_index = 0;
            all_pipes[i].buffer_reading_index = 0;
            number_of_existing_pipes++;
            return i + 3;
        }
    }
    return -1;
}

static uint8_t destroy_pipe(uint8_t pipe_ID) {
    if(number_of_existing_pipes == 0)
        return 1;

    uint8_t i;
    uint16_t j;
    for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
        if(all_pipes[i].pipe_unique_ID == pipe_ID) {
            all_pipes[i].pipe_unique_ID = 0;
            for(j=0; j<BUFFER_SIZE; j++)
                all_pipes[i].buffer[j] = 0;
            number_of_existing_pipes--;
            return 0;
        }
    }
    return 1;
}

void write_to_pipe(uint8_t pipe_index, char* to_write) {
    uint8_t real_pipe_index = pipe_index - 3;  // we have to substract the 3 that we added when we created the pipe
    uint8_t i = 0, writing_index;
    while(to_write[i] != '\0') {
        writing_index = all_pipes[real_pipe_index].buffer_writing_index;
        all_pipes[real_pipe_index].buffer_writing_index ++ ;
        all_pipes[real_pipe_index].buffer[writing_index] = to_write[i++];
        sem_post(all_pipes[real_pipe_index].semaphore_ID, 
                 all_pipes[real_pipe_index].writing_process_pid);
    }
    all_pipes[real_pipe_index].buffer[writing_index + 1] = '\0';
    sem_post(all_pipes[real_pipe_index].semaphore_ID, 
             all_pipes[real_pipe_index].writing_process_pid);
}

char read_letter_from_pipe(uint8_t pipe_index) {

    uint8_t real_pipe_index = pipe_index - 3;  // we have to substract the 3 that we added when we created the pipe
    uint8_t reading_index, current_letter, ret;

    ret = sem_wait(all_pipes[real_pipe_index].semaphore_ID, 
             all_pipes[real_pipe_index].reading_process_pid);
    if(ret == 1 || ret == 2)
        return -2;

    reading_index = all_pipes[real_pipe_index].buffer_reading_index;
    all_pipes[real_pipe_index].buffer_reading_index++;

    current_letter = all_pipes[real_pipe_index].buffer[reading_index];
    if(current_letter == '\0'){
        destroy_pipe(all_pipes[real_pipe_index].pipe_unique_ID);
        return -1;
    } else 
        return current_letter;
}

void list_all_pipes() {
    uint8_t i;
    if(number_of_existing_pipes == 0) {
        print(STD_OUTPUT, "There are no pipes open\n");
        return;
    }
    
    print(STD_OUTPUT, "PIPE_ID     WRITING_PROCESS_PID   READING_PROCESS_PID\n");
    for(i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
        if(all_pipes[i].pipe_unique_ID != 0) {
            print(STD_OUTPUT, num_to_string(all_pipes[i].pipe_unique_ID));
            print(STD_OUTPUT, "                ");
            print(STD_OUTPUT, num_to_string(all_pipes[i].writing_process_pid));
            print(STD_OUTPUT, "                ");
            print(STD_OUTPUT, num_to_string(all_pipes[i].reading_process_pid));
            print(STD_OUTPUT, "\n");
        }
    }
}