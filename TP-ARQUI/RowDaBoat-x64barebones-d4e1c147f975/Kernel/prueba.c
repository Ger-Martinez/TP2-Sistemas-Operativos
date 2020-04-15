#include "MemoryManager2.h"

int main(void) {
    char* vec[100];
    
    for(int i=0; i<100; i++) {
        vec[i] = malloc((uint64_t)1000);  // hago 100 malloc, cada uno con 1000 bytes
    }

    for(int i=0; i<50; i++) {
        free(vec[i*2]); // libero los pares
    }

    for(int i=0; i<50; i++) {
        free(vec[(i*2)+1]);  // libero los impares
    }
    return 0;
}