/* _loader.c */
#include <stdint.h>
#include "stdio.h"
#include "string.h"

extern char bss;
extern char endOfBinary;

///////////////////
extern char* num_to_string(uint8_t);
///////////////////

int main();
void * memset(void * destiny, int32_t c, uint64_t length);

int _start(uint8_t argc, uint8_t pid_key) {

	char* to_print = num_to_string(argc);
	print(to_print);
	char* b = num_to_string(pid_key);
	print(b);



	//Clean BSS
	memset(&bss, 0, &endOfBinary - &bss);

	return main();
}


void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}
