#include <stdint.h>

void* free_list_MALLOC(uint64_t wantedSize);
uint8_t free_list_FREE( void *addressToFree );
uint64_t free_list_MEM_STATE();

// esta la cree para probar. Quizas se podria borrar mas tarde
int countFreeBlocks(void);