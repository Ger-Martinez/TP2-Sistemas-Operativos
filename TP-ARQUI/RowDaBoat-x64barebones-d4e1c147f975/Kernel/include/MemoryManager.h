#include <stdint.h>

void* malloc(uint64_t wantedSize);
void free( void *addressToFree );

// esta la cree para probar. Quizas se podria borrar mas tarde
int countFreeBlocks(void);