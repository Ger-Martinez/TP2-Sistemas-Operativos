#include <stdint.h>
#include <screen_driver.h>
#include "lib.h"  // for NULL constant

#define MINIMUM_BLOCK_SIZE  4096  // 4KB

static uint64_t freeBytesRemaining = 0;
static uint16_t numberOfSuccessfulAllocations = 0;
static uint16_t numberOfSuccessfulFrees = 0;

// struct that represents a chunck of free space
typedef struct freeBlockNode {
    struct freeBlockNode * nextFreeBlock;
    uint64_t blockSize;
} freeBlockNode;

static void heapInit();
static void insertBlockIntoFreeList( freeBlockNode* blockToInsert );

static const uint16_t struct_size = sizeof(freeBlockNode);

static freeBlockNode freeListStart, *freeListEnd = NULL;

void* free_list_MALLOC(uint64_t wantedSize) {
	if(wantedSize <= 0)
		return NULL;
	
    freeBlockNode *returnedBlock, *previousBlock, *newFreeBlock; // only used if the block returned can be split into two
    void* returnAddress = NULL;
	
    /* if this is the first call to malloc, then the heap has to be initialised with the list of free blocks */
    if( freeListEnd == NULL )
        heapInit();

    /* The wanted size is increased so it can contain a freeBlockNode structure in addition to the requested amount of bytes. */
	wantedSize += struct_size;

    if(wantedSize <= freeBytesRemaining) {
		/* traverse the list until the FIRST big enough free block is found */
		previousBlock = &freeListStart;
		returnedBlock = freeListStart.nextFreeBlock;
		while( ( returnedBlock->blockSize < wantedSize ) && ( returnedBlock->nextFreeBlock != NULL ) ) {
			previousBlock = returnedBlock;
			returnedBlock = returnedBlock->nextFreeBlock;
		}

        /* If the end marker was reached then a block of adequate size was not found. */
		if( returnedBlock != freeListEnd ) {

			/* we return the address of the start of the chunck of free memory
				which is right after the struct */
			returnAddress = (void*) ( ((uint64_t)previousBlock->nextFreeBlock) + struct_size );

            /* This block is being returned for use so must be taken out of the list of free blocks. */
			previousBlock->nextFreeBlock = returnedBlock->nextFreeBlock;

            /* If the block is larger than required it can be split into two. */
			if( ( returnedBlock->blockSize - wantedSize ) > MINIMUM_BLOCK_SIZE ) {

				/* Create a new block right after the end of the chuck of free space returned. */
				newFreeBlock = (void*) ( ((uint64_t*)returnedBlock) + wantedSize );

                /* Calculate the sizes of the two blocks split from the single block. */
				newFreeBlock->blockSize = returnedBlock->blockSize - wantedSize;
				returnedBlock->blockSize = wantedSize - struct_size;

				/* Insert the new block into the list of free blocks. */
				insertBlockIntoFreeList( newFreeBlock );
            }

            freeBytesRemaining -= returnedBlock->blockSize;
			freeBytesRemaining -= struct_size; // always remember that the structs ocuppy space in memory

			returnedBlock->nextFreeBlock = NULL;
			numberOfSuccessfulAllocations++;
        }
		else{
			return NULL;
		}
    }
	else{
		return NULL;
	}
	return returnAddress;
}

uint8_t free_list_FREE( void *addressToFree ) {
	if(addressToFree != NULL) {
		uint64_t numerical_address = (uint64_t) addressToFree;
		numerical_address -= struct_size;  // the struct representing the block is right before the block
		freeBlockNode* to_free = (freeBlockNode*)numerical_address;
		freeBytesRemaining += to_free->blockSize;
		numberOfSuccessfulFrees++;
		insertBlockIntoFreeList( ((freeBlockNode*)to_free) );
		return 0;
	}
	return 1;
}

static void insertBlockIntoFreeList( freeBlockNode* blockToInsert ){
	freeBlockNode* nodeIterator;
	uint64_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( nodeIterator = &freeListStart; nodeIterator->nextFreeBlock < blockToInsert; nodeIterator = nodeIterator->nextFreeBlock ) {
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after make a contiguous block of memory? */
	puc = (uint64_t*) nodeIterator;
	if( ( puc + nodeIterator->blockSize ) == (uint64_t*) blockToInsert ) {
		nodeIterator->blockSize += blockToInsert->blockSize;
		blockToInsert = nodeIterator;
	}

	/* Do the block being inserted, and the block it is being inserted before make a contiguous block of memory? */
	puc = (uint64_t*) blockToInsert;
	if( ( puc + blockToInsert->blockSize ) == (uint64_t*) nodeIterator->nextFreeBlock ) {
		if( nodeIterator->nextFreeBlock != freeListEnd ) {
			/* Form one big block from the two blocks. */
			blockToInsert->blockSize += nodeIterator->nextFreeBlock->blockSize;
			blockToInsert->nextFreeBlock = nodeIterator->nextFreeBlock->nextFreeBlock;
		}
		else {
			blockToInsert->nextFreeBlock = freeListEnd;
		}
	}
	else {
		blockToInsert->nextFreeBlock = nodeIterator->nextFreeBlock;
	}

	if( nodeIterator != blockToInsert ) {
		nodeIterator->nextFreeBlock = blockToInsert;
	}
}

static void heapInit() {
    freeBlockNode* firstFreeBlock;
    uint64_t *addrAlignedHeap = (void*) 0x600000;  // desde los 6 megas
    uint64_t heap_end = 0x8600000; // hasta los 134 megas

    /* freeListStart is used to hold a pointer to the first item in the list of free blocks.  
		The void cast is used to prevent compiler warnings. */
    freeListStart.nextFreeBlock = (void*) addrAlignedHeap;
    freeListStart.blockSize = (uint16_t) 0;

    /* freeListEnd is used to mark the end of the list of free blocks and is inserted at the end of the memory */
	freeListEnd = (void*) heap_end;
	freeListEnd->blockSize = 0;
	freeListEnd->nextFreeBlock = NULL;

    /* To start with, there is a single free block that is sized to take up the
	entire heap space, minus the space taken by freeListEnd. */
	firstFreeBlock = (void*) addrAlignedHeap;
	firstFreeBlock->blockSize = heap_end - (uint64_t) firstFreeBlock - sizeof(freeBlockNode);
	firstFreeBlock->nextFreeBlock = freeListEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
	freeBytesRemaining = firstFreeBlock->blockSize;
}

int countFreeBlocks() {
	freeBlockNode* iterator;
	int count = 0;
	for(iterator = &freeListStart; iterator->nextFreeBlock != freeListEnd; iterator = iterator->nextFreeBlock) {
		count++;
	}
	return count;
}

uint64_t free_list_MEM_STATE() {
	return freeBytesRemaining;
}