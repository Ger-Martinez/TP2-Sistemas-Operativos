#ifndef TEST_SYNCHRO_H
#define TEST_SYNCHRO_H

#include <stdint.h>

uint8_t test_sync(uint8_t background, uint8_t pid_key);
uint8_t test_no_sync(uint8_t background, uint8_t pid_key);

#endif