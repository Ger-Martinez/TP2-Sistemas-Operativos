#include <stdint.h>

#define MIN_BLOCK_SIZE 4096  // 4KB
#define TOTAL_HEAP_SIZE (128 * 1024 * 1024)  // 128 MB
#define NUMBER_OF_ARRAYS (TOTAL_HEAP_SIZE / MIN_BLOCK_SIZE + 1)
#define NULL ((void*)0)

char blocks[NUMBER_OF_ARRAYS][];

uint8_t initialized = 0;

void initialize() {
    
    for (int i = 0; i < NUMBER_OF_ARRAYS; i++) {
        int size = pow(2, i);
        
        char aux [size];
        int j = 0;
        // while (j < size) {
        //     aux[j] = 0;
        // }

        blocks[size] = aux;
    }
}

void * malloc (uint8_t bytes) {

    if (bytes > TOTAL_HEAP_SIZE)
        return NULL;
    
    if (initialized == 0) {
        initialized = 1;
        initialize();
    }
    
}

/* Simple pow function for mathematical porpouses */
uint64_t pow( int base, int exp ) {
    int i = 1;
    int aux = base;
    while (i < exp) {
        aux *= base;
        i++;
    }
    return aux;
}

/*void* malloc(uint64_t wantedSize) {

}*/ 



[] [] [] [] [] [] [] [] [] [] [] [] [] [] [] []

[]    []    []    []    []    []    []    []

[]          []          []          []

[]                      []

[]