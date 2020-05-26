// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* _loader.c */
#include <stdint.h>
#include "stdio.h"
#include "string.h"

extern char bss;
extern char endOfBinary;

int main();

int _start(uint8_t pid_key) {
	//Clean BSS
	memset(&bss, 0, &endOfBinary - &bss);

	return main(pid_key);
}