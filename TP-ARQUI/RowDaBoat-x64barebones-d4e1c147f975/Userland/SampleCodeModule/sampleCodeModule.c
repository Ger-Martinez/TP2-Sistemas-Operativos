#include "stdio.h"
#include "shell.h"
#include <stdint.h>

int main(uint8_t pid_key) {
	start_shell(pid_key);
	return 0;
}