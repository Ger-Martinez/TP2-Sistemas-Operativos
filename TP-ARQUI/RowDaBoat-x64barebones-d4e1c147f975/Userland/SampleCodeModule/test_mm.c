#include <stdio.h>
#include <string.h>
#include "test_util.h"

#define MAX_BLOCKS 128
#define MAX_MEMORY 106954752  // 102 MB, around 80% of total heap = 128 MB

extern void* syscall_malloc(uint64_t);
extern uint64_t syscall_free(void*);

typedef struct MM_rq{
    void *address;
    uint32_t size;
}mm_rq;

void test_mm(){
    mm_rq mm_rqs[MAX_BLOCKS];
    uint8_t rq;
    uint32_t total;

    while (1){
        rq = 0;
        total = 0;

        // Request as many blocks as we can
        while(rq < MAX_BLOCKS && total < MAX_MEMORY){
            mm_rqs[rq].size = GetUniform(MAX_MEMORY - total - 1) + 1;
            mm_rqs[rq].address = syscall_malloc(mm_rqs[rq].size); // TODO: Port this call as required

            total += mm_rqs[rq].size;
            rq++;
        }

        // Set
        uint32_t i;
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address != NULL)
                memset(mm_rqs[i].address, i, mm_rqs[i].size); // TODO: Port this call as required

        // Check
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address != NULL)
                if(!memcheck(mm_rqs[i].address, i, mm_rqs[i].size))
                     print("ERROR!\n"); // TODO: Port this call as required

        // Free
        for (i = 0; i < rq; i++)
            if (mm_rqs[i].address != NULL)
                syscall_free(mm_rqs[i].address);  // TODO: Port this call as required
    } 
}