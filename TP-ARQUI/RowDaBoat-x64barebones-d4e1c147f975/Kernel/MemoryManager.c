#include <stdint.h>
#include <screen_driver.h>
#define NULL ((void*)0)

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE  512  // 0x200

/* Keeps track of the number of calls to allocate and free memory as well as the
number of free bytes remaining, but says nothing about fragmentation. */
static uint64_t freeBytesRemaining = 0;
static uint16_t numberOfSuccessfulAllocations = 0;
static uint16_t numberOfSuccessfulFrees = 0;

/* Gets set to the top bit of an uint16_t type.  When this bit in the xBlockSize
member of an freeBlockNode structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static uint16_t xBlockAllocatedBit = 0;


typedef struct freeBlockNode {
    struct freeBlockNode * nextFreeBlock;
    uint64_t blockSize;
} freeBlockNode;

static void heapInit();
static void prvInsertBlockIntoFreeList( freeBlockNode* blockToInsert );

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const uint16_t xHeapStructSize = sizeof(freeBlockNode);/*+ ( ( uint16_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( uint16_t ) portBYTE_ALIGNMENT_MASK );*/

static freeBlockNode freeListStart, *freeListEnd = NULL;

void* malloc(uint64_t wantedSize) {
    freeBlockNode *block1, *previousBlock, *newFreeBlock;
    void* returnAddress = NULL;
	
    /* if this is the first call to malloc, then the heap has to be initialised with the list of free blocks */
    if( freeListEnd == NULL ) {
        heapInit();
    }
    /* Check the requested block size is not so large that the top bit is
	set.  The top bit of the block size member of the freeBlockNode structure
	is used to determine who owns the block - the application or the
	kernel, so it must be free. */

	//if( ( wantedSize & xBlockAllocatedBit ) == 0 ) {

        /* The wanted size is increased so it can contain a freeBlockNode
		structure in addition to the requested amount of bytes. */
		if( wantedSize > 0 ) {
			wantedSize += xHeapStructSize;
		}

        if( ( wantedSize > 0 ) && ( wantedSize <= freeBytesRemaining ) ) {
			/* Traverse the list from the start	(lowest address) block until
			one	of adequate size is found. */ /* aca se ve el First Fit */
			previousBlock = &freeListStart;
			block1 = freeListStart.nextFreeBlock;
			while( ( block1->blockSize < wantedSize ) && ( block1->nextFreeBlock != NULL ) ) {
				drawString("XXXXXXX    "); // NO DEBERIA IMPRIMIRSE LA PRIMERA VEZ
				previousBlock = block1;
				block1 = block1->nextFreeBlock;
			}

            /* If the end marker was reached then a block of adequate size
			was	not found. */
			if( block1 != freeListEnd ) {
				/* Return the memory space pointed to - jumping over the
				freeBlockNode structure at its start. */
				// drawNumber(((uint64_t*)previousBlock->nextFreeBlock), 0xFFFFFF, 0x000000); /* 6MB */
				// drawString(" + ");
				// drawNumber(xHeapStructSize, 0xFFFFFF, 0x000000); /* 16 BYTES */
				// drawString("  = ");
				// drawNumber(((uint64_t*)previousBlock->nextFreeBlock) + xHeapStructSize, 0xFFFFFF, 0x000000); /* 6mb + 128 bytes */
				returnAddress = (void*) ( ((uint64_t*)previousBlock->nextFreeBlock) + xHeapStructSize );

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
					//configASSERT( ( ( ( uint16_t ) newFreeBlock ) & portBYTE_ALIGNMENT_MASK ) == 0 );

                    /* Calculate the sizes of two blocks split from the
					single block. */
					newFreeBlock->blockSize = block1->blockSize - wantedSize;
					block1->blockSize = wantedSize;

					/* Insert the new block into the list of free blocks. */
					prvInsertBlockIntoFreeList( newFreeBlock );
                }

                freeBytesRemaining -= block1->blockSize;

                /* The block is being returned - it is allocated and owned
				by the application and has no "next" block. */
				//block1->blockSize |= xBlockAllocatedBit;
				block1->nextFreeBlock = NULL;
				numberOfSuccessfulAllocations++;
            }
        }
    //}
	return returnAddress;
}

void free( void *addressToFree ) {
    uint64_t* addr = (uint64_t*) addressToFree;
    freeBlockNode *block1;

	if( addressToFree != NULL ) {
		/* The memory being freed will have an freeBlockNode structure immediately
		before it. */
		addr -= xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		block1 = (void*) addr;

		//if( ( block1->blockSize & xBlockAllocatedBit ) != 0 ) {
			if( block1->nextFreeBlock == NULL ) {
				/* The block is being returned to the heap - it is no longer
				allocated. */
				//block1->blockSize &= ~xBlockAllocatedBit;

				/* Add this block to the list of free blocks. */
				freeBytesRemaining += block1->blockSize;
				prvInsertBlockIntoFreeList( ((freeBlockNode*)block1) );
				numberOfSuccessfulFrees++;
            }
        //}
    }
}

static void prvInsertBlockIntoFreeList( freeBlockNode* blockToInsert ){
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

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( nodeIterator != blockToInsert ) {
		nodeIterator->nextFreeBlock = blockToInsert;
	}
}

static void heapInit() {
    freeBlockNode* firstFreeBlock;
    uint64_t *addrAlignedHeap = (void*) 0x600000;  // desde los 6 megas
    uint64_t uxAddress = 0x8000000; // hasta los 128 megas
	drawString("entre a heapInit   ");
	/*drawNumber(uxAddress, 0xFFFFFF, 0x000000);
	drawString("      ");*/

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
	/* el primer bloque tendrá 122 MB de espacio */
	firstFreeBlock->blockSize = uxAddress - (uint64_t) firstFreeBlock/* - sizeof(struct freeBlockNode)*/;
	firstFreeBlock->nextFreeBlock = freeListEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
	freeBytesRemaining = firstFreeBlock->blockSize;
}
