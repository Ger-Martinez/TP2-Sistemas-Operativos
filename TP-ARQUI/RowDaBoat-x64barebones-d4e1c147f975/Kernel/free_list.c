#include <stdint.h>
#include <screen_driver.h>
#include "lib.h"  // for NULL constant

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE  4096  // 4KB

/* Keeps track of the number of calls to allocate and free memory as well as the
number of free bytes remaining, but says nothing about fragmentation. */
static uint64_t freeBytesRemaining = 0;
static uint16_t numberOfSuccessfulAllocations = 0;
static uint16_t numberOfSuccessfulFrees = 0;

typedef struct freeBlockNode {
    struct freeBlockNode * nextFreeBlock;
    uint64_t blockSize;
} freeBlockNode;

static void heapInit();
static void insertBlockIntoFreeList( freeBlockNode* blockToInsert );

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const uint16_t xHeapStructSize = sizeof(freeBlockNode);

static freeBlockNode freeListStart, *freeListEnd = NULL;

void* free_list_MALLOC(uint64_t wantedSize) {
	//drawNumber(wantedSize, 0xFFFFFF, 0x000000);  drawString("   \n");
	if(wantedSize <= 0)
		return NULL;
	
    freeBlockNode *block1, *previousBlock, *newFreeBlock;
    void* returnAddress = NULL;
	
    /* if this is the first call to malloc, then the heap has to be initialised with the list of free blocks */
    if( freeListEnd == NULL )
        heapInit();

    /* The wanted size is increased so it can contain a freeBlockNode
	structure in addition to the requested amount of bytes. */
	wantedSize += xHeapStructSize;

    if(wantedSize <= freeBytesRemaining) {
		/* traverse the list until the FIRST big enough free block is found */
		previousBlock = &freeListStart;
		block1 = freeListStart.nextFreeBlock;
		while( ( block1->blockSize < wantedSize ) && ( block1->nextFreeBlock != NULL ) ) {
			previousBlock = block1;
			block1 = block1->nextFreeBlock;
		}

        /* If the end marker was reached then a block of adequate size was not found. */
		if( block1 != freeListEnd ) {
			/* returnaddress = address of block + sizeof(block) */
			returnAddress = (void*) ( ((uint64_t)previousBlock->nextFreeBlock) + xHeapStructSize );

            /* This block is being returned for use so must be taken out
			of the list of free blocks. */
			previousBlock->nextFreeBlock = block1->nextFreeBlock;

            /* If the block is larger than required it can be split into two. */
			if( ( block1->blockSize - wantedSize ) > heapMINIMUM_BLOCK_SIZE ) {
				/* This block is to be split into two.  Create a new
				block following the number of bytes requested. The void
				cast is used to prevent byte alignment warnings from the
				compiler. */
				newFreeBlock = (void*) ( ((uint64_t*)block1) + wantedSize );

                /* Calculate the sizes of two blocks split from the
				single block. */

				newFreeBlock->blockSize = block1->blockSize - wantedSize;
				block1->blockSize = wantedSize - xHeapStructSize;

				/* Insert the new block into the list of free blocks. */
				insertBlockIntoFreeList( newFreeBlock );
            }

            freeBytesRemaining -= block1->blockSize;
			freeBytesRemaining -= xHeapStructSize;

            /* The block is being returned - it is allocated and owned
			by the application and has no "next" block. */
			block1->nextFreeBlock = NULL;
			numberOfSuccessfulAllocations++;
        }
		else{
			//drawString("  NO BLOCKS   ");
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
		numerical_address -= xHeapStructSize;
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

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = (uint64_t*) nodeIterator;
	if( ( puc + nodeIterator->blockSize ) == (uint64_t*) blockToInsert ) {
		nodeIterator->blockSize += blockToInsert->blockSize;
		blockToInsert = nodeIterator;
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
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
    uint64_t uxAddress = 0x8600000; // hasta los 134 megas

    /* freeListStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
    freeListStart.nextFreeBlock = (void*) addrAlignedHeap;
    freeListStart.blockSize = (uint16_t) 0;

    /* freeListEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	freeListEnd = (void*) uxAddress;
	freeListEnd->blockSize = 0;
	freeListEnd->nextFreeBlock = NULL;

    /* To start with, there is a single free block that is sized to take up the
	entire heap space, minus the space taken by freeListEnd. */
	firstFreeBlock = (void*) addrAlignedHeap;
	/* el primer bloque tendrá 128 MB - sizeof(struct) de espacio */
	firstFreeBlock->blockSize = uxAddress - (uint64_t) firstFreeBlock - sizeof(freeBlockNode);
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