#include "string.h"
#include "stdio.h"
#include "pipes.h"
#include <stdint.h>

extern void syscall_write(char* toWrite, int length);
extern void syscall_read(int file_descriptor, char* toRead);
////////////////
extern char* num_to_string(int num);
////////////

void print(uint8_t where_to_write, char* string) {
    if(where_to_write == STD_OUTPUT || where_to_write == STD_ERR)
        syscall_write(string, strlen(string));
    else
        write_to_pipe(where_to_write, string);
}

void putchar(uint8_t where_to_write, char c) {
    char to_write[] = {'\0', '\0'};
    to_write[0] = c;
    print(where_to_write, to_write);
}

char getChar(uint8_t where_to_read) {
    char resp;
    if(where_to_read == STD_INPUT)
        syscall_read(STD_INPUT, &resp);
    else
        resp = read_letter_from_pipe(where_to_read);
    return resp;
}

uint8_t read(uint8_t where_to_read, char* buffer) {
    char c;
    uint8_t i = 0;
    if(where_to_read != STD_INPUT){

        do {
            c = read_letter_from_pipe(where_to_read);
            if(c == -2)
                return 1;
            else if(c == -1) 
                *(buffer + (i * sizeof(char))) = '\0';
            else {
                *(buffer + (i * sizeof(char))) = c;
                i++;
            }
        } while (c != -1);
        return 0;

    } else {
        // there is no implementation (yet) for reading a whole sentence from STD_INPUT
        // you have to use getChar, and read letter by letter
        return 1;
    }
}