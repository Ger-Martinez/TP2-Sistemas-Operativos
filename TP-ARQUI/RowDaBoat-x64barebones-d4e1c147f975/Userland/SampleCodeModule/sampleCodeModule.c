// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdio.h"
#include "shell.h"
#include <stdint.h>

int main(uint8_t pid_key) {
	start_shell(pid_key);
	return 0;
}