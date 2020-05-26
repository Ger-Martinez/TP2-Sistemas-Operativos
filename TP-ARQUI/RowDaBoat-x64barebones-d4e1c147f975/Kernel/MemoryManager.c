// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "buddy.h"
#include "free_list.h"

void* malloc(uint64_t wantedSize) {

    // THIS INTERFACE RETURNS THE FREE-LIST MALLOC
    
    return free_list_MALLOC(wantedSize);
    //return buddy_MALLOC(wantedSize);
}

uint8_t free(void* address) {

    // THIS INTERFACE RETURNS THE FREE-LIST FREE

    return free_list_FREE(address);
}

uint64_t mem_state() {

    // THIS INTERFACE RETURNS THE FREE-LIST MEM_STATE
    return free_list_MEM_STATE();
    //return buddy_MEM_STATE();
}