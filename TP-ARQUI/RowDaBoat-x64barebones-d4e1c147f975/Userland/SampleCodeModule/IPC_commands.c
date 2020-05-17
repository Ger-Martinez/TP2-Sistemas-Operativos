#include <stdint.h>
#include "stdio.h"

extern uint64_t syscall_create_process(uint64_t, int, uint8_t, uint8_t);
extern char* num_to_string(int number);
extern uint64_t syscall_read(int, char*, int);
extern uint64_t syscall_exit(uint8_t pid_key);
static uint8_t isNotVocal(char c);

// process fg
static void fg_code(uint8_t pid_key, uint8_t where_to_write) {
    char buffer[128 * 3] = {'\0'};
    uint8_t k = 0;
    char c;
    while(1){
        c = getChar(STD_INPUT);
        if(c == '_'){  // simula un '\0'
            buffer[k] = '\0';
            print(where_to_write, buffer);
            syscall_exit(pid_key);
        }
        else{
            putchar(STD_OUTPUT, c);
            buffer[k++] = c;
        }
    }
}

// command fg
void fg(uint8_t pid_key, uint8_t where_to_write) {
    void (*foo)(uint8_t, uint8_t);
    foo  = fg_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 0, pid_key, where_to_write);
    if(ret == 1)
        print(STD_ERR, "ERROR AL CREAR PROCESS_A\n");
}

// process wc
static void wc_code(uint8_t pid_key, uint8_t where_to_read) {
    char buffer[128 * 3] = {'\0'};
    uint8_t ret, count_of_lines = 0;
    uint16_t i;
    ret = read(where_to_read, &(buffer[0]));
    if(ret == 1) {
        print(STD_ERR, "hubo error en el read\n");
        syscall_exit(pid_key);
    }
    print(STD_OUTPUT, "\n WC returns:\n");

    for(i = 0; buffer[i] != '\0'; i++)
        if(buffer[i] == '\n')
            count_of_lines++;
    
    print(STD_OUTPUT, num_to_string(count_of_lines));
    syscall_exit(pid_key);
}

// command wc
void wc(uint8_t pid_key, uint8_t where_to_read) {
    void (*foo)(uint8_t, uint8_t);
    foo = wc_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 1, pid_key, where_to_read);
    if(ret == 1)
        print(STD_ERR, "ERROR AL CREAR PROCESO\n");
}

// process cat
static void cat_code(uint8_t pid_key, uint8_t where_to_read) {
    char buffer[128 * 3] = {'\0'};
    uint8_t ret;
    ret = read(where_to_read, &(buffer[0]));
    if(ret == 1) {
        print(STD_ERR, "hubo error en el read\n");
        syscall_exit(pid_key);
    }
    print(STD_OUTPUT, "\n CAT returns:\n");
    print(STD_OUTPUT, buffer);
    syscall_exit(pid_key);
}

// command cat
void cat(uint8_t pid_key, uint8_t where_to_read) {
    void (*foo)(uint8_t, uint8_t);
    foo = cat_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 1, pid_key, where_to_read);
    if(ret == 1)
        print(STD_ERR, "ERROR AL CREAR PROCESO\n");
}

// process filter
static void filter_code(uint8_t pid_key, uint8_t where_to_read) {
    char buffer[128 * 3] = {'\0'};
    char buffer_no_vocals[128 * 3] = {'\0'};
    uint8_t ret;
    uint16_t i, j;
    ret = read(where_to_read, &(buffer[0]));
    if(ret == 1) {
        print(STD_ERR, "hubo error en el read\n");
        syscall_exit(pid_key);
    }
    print(STD_OUTPUT, "\n FILTER returns:\n");

    for(i = 0; buffer[i] != '\0'; i++) {
        if(isNotVocal(buffer[i])) {
            buffer_no_vocals[j++] = buffer[i];
        }
    }
    buffer_no_vocals[j] = '\0';
    print(STD_OUTPUT, buffer_no_vocals);
    syscall_exit(pid_key);
}

// command filter
void filter(uint8_t pid_key, uint8_t where_to_read) {
    void (*foo)(uint8_t, uint8_t);
    foo = filter_code;
    uint32_t ret = syscall_create_process((uint64_t)foo, 1, pid_key, where_to_read);
    if(ret == 1)
        print(STD_ERR, "ERROR AL CREAR PROCESO\n");
}

static uint8_t isNotVocal(char c) {
    if(c!='a' && c!='A' && c!='e' && c!='E' && c!='i' && c!='I' && c!='o' && c!='O' && c!='u' && c!='U') {
        return 1;
    }
    return 0;
}