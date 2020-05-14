#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>

#define NULL ((void*)0)
#define STD_INPUT 0
#define STD_OUTPUT 1
#define STD_ERR 2

void print(uint8_t where_to_write, char* string);
char getChar(uint8_t where_to_read);
void putchar(uint8_t where_to_write, char c);
uint8_t read(uint8_t where_to_read, char* buffer);

#endif