// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>

// CODIGO SACADO DE INTERNET
int strcmp (const char *p1, const char *p2) {
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;
    do{
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    }while (c1 == c2);
    return c1 - c2;
}

int strlen(char* string) {
    int length = 0;
    for(int i=0; string[i]!='\0'; i++) {
        length++;
    }
    return length;
}


int string_to_num(char* parameter) {
    int length = strlen(parameter);
    int multiplier = 1;
    int result = 0;
    if(length == 0) {
        return -1;
    }
    while(length >= 1) {
        result += (parameter[length-1] - 0x30) * multiplier;
        multiplier *= 10;
        length--;
    }
    return result;
}

void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}