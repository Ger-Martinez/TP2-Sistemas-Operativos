#include "buddy.h"
#include "free_list.h"

void* malloc(uint64_t wantedSize) {

    // POR AHORA RETORNA EL malloc DE FREE_LIST
    
    return free_list_MALLOC(wantedSize);
    //return buddy_MALLOC(wantedSize);
}

void free(void* address) {

    // POR AHORA RETORNA EL free DE FREE_LIST

    return free_list_FREE(address);
}

uint64_t mem_state() {

    // POR AHORA RETORNA EL mem_state DE FREE_LIST
    return free_list_MEM_STATE();
    //return buddy_MEM_STATE();
}