#include <stdint.h>
#define MINIMIN_BLOCK_SIZE 4096  // 4KB
#define TOTAL_HEAP_SIZE (128 * 1024 * 1024)  // 128 MB

// 0 --> unoccupied    1--> occupied
uint8_t occupied_blocks[TOTAL_HEAP_SIZE / MINIMIN_BLOCK_SIZE] = {0};

/*void* malloc(uint64_t wantedSize) {

}*/