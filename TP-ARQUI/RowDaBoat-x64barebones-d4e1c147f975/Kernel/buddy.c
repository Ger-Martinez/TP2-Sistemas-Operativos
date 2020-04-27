#include <stdint.h>
#include <screen_driver.h>


/***********  STILL UNDER DEVELOPMENT  **************/


#define MIN_BLOCK_SIZE 4096  // 4KB
#define TOTAL_HEAP_SIZE (128 * 1024 * 1024)  // 128 MB
#define START_OF_HEAP (void*) 0x600000
#define END_OF_HEAP (void*) 0x8600000
#define NUMBER_OF_ARRAYS (TOTAL_HEAP_SIZE / MIN_BLOCK_SIZE + 1)
#define NULL ((void*)0)

#define FREE ('F') 
#define OCCUPED ('O') 
#define LEFT_OCCUPED ('L') 
#define RIGHT_OCCUPED ('R') 

#define LEFT ('l') 
#define RIGHT ('r') 


uint8_t initialized = 0;

typedef struct buddyHeader {
    uint64_t size;
    uint8_t status;
    uint8_t position;
    /* void* address; */
} buddyHeader;

buddyHeader * firstHeader = START_OF_HEAP;
buddyHeader * currentHeader;
buddyHeader * nextHeader;
 

void initialize() {
    buddyHeader * bh = (void *) START_OF_HEAP;
    bh->size = TOTAL_HEAP_SIZE - sizeof(buddyHeader);
    bh->status = FREE;
    bh->position = LEFT;
}

void * buddy_MALLOC (uint64_t bytesRequested) {

    if (bytesRequested > TOTAL_HEAP_SIZE - sizeof(buddyHeader))
        return NULL;
            
    if (initialized == 0) {
        initialize();
        initialized = 1;
    }

    currentHeader = firstHeader;
    nextHeader = currentHeader + sizeof(currentHeader) + currentHeader->size;
    // drawNumber(nextHeader, 0xFFFFFF, 0x000000);
    buddyHeader * bestFit = NULL;
    uint64_t aux;
    uint8_t found = 0;

    while(nextHeader < firstHeader + TOTAL_HEAP_SIZE) {
        if((bestFit == NULL && bytesRequested <= currentHeader->size && currentHeader->status == FREE) || 
            (bytesRequested <= currentHeader->size && currentHeader->size < bestFit->size  && currentHeader->status == FREE)) {
                bestFit = currentHeader;
                found = 1;
        }
        aux = currentHeader->size + sizeof(buddyHeader);
        currentHeader = nextHeader;
        nextHeader += aux;
    }

    
    if(found == 0)
        return NULL;

    uint64_t newSize;

    while(bestFit->size / 2 - sizeof(buddyHeader) >= bytesRequested && (bestFit->size + sizeof(buddyHeader)) >= MIN_BLOCK_SIZE) {
        newSize = (bestFit->size + sizeof(buddyHeader)) / 2 - sizeof(buddyHeader);
        bestFit->size = newSize;
        bestFit->status = OCCUPED;

        buddyHeader * newHeader = bestFit + newSize + sizeof(buddyHeader); 
        newHeader->status = FREE;
        newHeader->size = newSize;
    }

    return bestFit + sizeof(buddyHeader);
}
