#include <screen_driver.h>
//#include <time.h>   /* may be implemented in the future */
#include <keyboard.h>
#include <stdint.h>
#include "lib.h"
#include "process_manager.h"
#include "MemoryManager.h"
#include "scheduler.h"

#define STD_INPUT 0
#define STD_OUTPUT 1
#define RTC 3
//#define TIMER_TICK 4    /* may be implemented in the future */

extern int segundos();
extern int minutos();
extern int horas();
extern void _hlt(void);
static uint64_t write_syscall(int file_descriptor, char* string);
static uint64_t read_syscall(int where_to_read, char* where_to_store);


uint64_t syscall_dispatcher(int ID, int second_parameter, char* third_parameter, int fourth_parameter, uint8_t fifth_parameter) {
    switch(ID){
        case 4:
            return write_syscall(second_parameter, third_parameter);
        case 3:
            return read_syscall(second_parameter, third_parameter);
        case 2:
            return create_process((uint64_t)second_parameter, (uint8_t)third_parameter, (uint8_t)fourth_parameter, fifth_parameter);
        case 45:
            return ( (uint64_t) malloc((uint64_t)second_parameter) );
        case 5:
            return free((void*)second_parameter);
        case 37:
            return kill_process(second_parameter);
        case 6:
            return mem_state();
        case 20:
            return getpid(second_parameter);
        case 1:{
            exit_process(second_parameter);
            return 0;  // the CPU will never get to this return
        }
        case 7:
            return negate_state(second_parameter, (uint16_t)third_parameter);
        case 8:
            return ps();
        case 9:
            return change_priority(second_parameter, (uint8_t)third_parameter);
        default:{
            // we just add this to keep warnings quiet
            return 0;
        }
    }
}

static uint64_t write_syscall(int file_descriptor, char* string) {
    switch(file_descriptor){
        case STD_OUTPUT:{
            drawString(string);
            return (uint64_t)strlen(string);
        }
        default:{
            // we just add this to keep warnings quiet
            return 0;
        }
    }
}

static uint64_t read_syscall(int where_to_read, char* where_to_store) {
    switch(where_to_read){
        case STD_INPUT:{  // read from the keyboard buffer
            if(get_keyboard_buffer() == -1) {
                // if the keyboard_buffer does NOT have a letter to return, we are in a BLOCKING READ-SYSCALL
                change_process_state_with_INDEX(get_foreground_process(), BLOCKED_READING);
                while(1){
                    if(!buffer_is_empty())
                        break;
                    _hlt();
                }
            }
            // we arrive here once the keyboard_buffer has a letter, and the process that made this READ is unblocked
            where_to_store[0] = get_keyboard_buffer();
            return 0; // success
        }
        case RTC:{
            /* Arqui legacy */
            int seg = segundos();
			int min = minutos();
			int hora = horas();
			where_to_store[0]=hora/10 + 0x30; 
            where_to_store[1]=hora%10 + 0x30;
			where_to_store[3]=min/10 + 0x30; 
            where_to_store[4]=min%10 + 0x30;
			where_to_store[6]=seg/10 + 0x30; 
            where_to_store[7]=seg%10 + 0x30;
			return 0; // success
        }
        default:{
            return 1; // error
        }
    }
}