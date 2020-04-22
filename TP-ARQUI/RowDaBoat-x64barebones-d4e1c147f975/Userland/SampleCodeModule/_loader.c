/* _loader.c */
#include <stdint.h>
#include "stdio.h"
#include "string.h"

extern char bss;
extern char endOfBinary;

int main();
void * memset(void * destiny, int32_t c, uint64_t length);

int _start(uint8_t pid_key) {

	//Clean BSS
	memset(&bss, 0, &endOfBinary - &bss);

	return main(pid_key);
}


void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}
