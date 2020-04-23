#include "stdio.h"
#include "shellCommands.h"
#include "string.h"
#include <stdint.h>

#define SHELL_BUFFER_SIZE ( (128)+(128/2) )  // 1 linea y media

static void shell_main(uint8_t pid_key);
static int search_command(char command_with_possible_parameter[], char* parameter, uint8_t* background);

static char shell_buffer[SHELL_BUFFER_SIZE] = {'\0'};
static uint8_t k = 0; // to move across the buffer

void start_shell(uint8_t pid_key) {
    print(">>");
    shell_main(pid_key);
}

static void shell_main(uint8_t pid_key) {
    char c;
    int command;
    while(1) {
        c = getChar();
        if(c != -1) {
            if(c == '\n') {
                putchar(c);     // print the '\n' to move to the next line
                shell_buffer[k++] = '\0';
                char* parameter = "X";
                uint8_t background = 0;
                command = search_command(shell_buffer, parameter, &background);
                if(command != -1)
                    execute_command(command, parameter, pid_key, background);
                else
                    print("   Command not found");
                putchar(c);  // print the '\n' to move to the next line
                // empty our shell_buffer to prepare for the next line
                for(int i=0; shell_buffer[i] != '\0'; i++) {
                    shell_buffer[i] = '\0';
                }
                k = 0;
                print(">>");
            }
            else if(c=='\b' && k>0) {
                shell_buffer[k--] = '\0';
                putchar(c);
            }
            else { // it is a normal character
                if(k < SHELL_BUFFER_SIZE) {
                    shell_buffer[k++] = c;
                    putchar(c);
                }
            }
        }
    }
}

static int search_command(char command_with_possible_parameter[], char* parameter, uint8_t* background) {
    
    int number_of_command = -1;
    char command_without_parameter[SHELL_BUFFER_SIZE] = {'\0'};

    uint8_t i=0;
    for( ; command_with_possible_parameter[i]!='\0' && command_with_possible_parameter[i]!=' '; i++){
        command_without_parameter[i] = command_with_possible_parameter[i];
    }
    command_without_parameter[i] = '\0';
    uint8_t command_found = 0;
    for(uint8_t j=0; j<NUMBER_OF_COMMANDS && !command_found; j++) {
        if( strcmp(command_without_parameter, all_commands[j]) == 0) {
            number_of_command = j;
            command_found = 1;
        }
    }

    if(number_of_command == -1) {
        // si no encontre el comando, ni me fijo en el parametro, y salgo de esta funcion
        return -1;
    }

    if(command_with_possible_parameter[i] == ' ') {
        // there is also a parameter next to the command, or maybe a '&'
        if(command_with_possible_parameter[i+1] == '&') {
            *background = 1;
            i++;
            if(command_with_possible_parameter[i+1] != '\0')
                return -1;

        } else {
            uint8_t j2=0, j1;
            for(j1=i+1; command_with_possible_parameter[j1]!='\0' && command_with_possible_parameter[j1] != ' '; j1++) {
                parameter[j2++] = command_with_possible_parameter[j1];
            }
            parameter[j2++] = '\0';
            if(command_with_possible_parameter[j1] == ' ') {
                if(command_with_possible_parameter[j1+1] == '&'){
                    j1++;
                    *background = 1;
                    if(command_with_possible_parameter[j1+1] != '\0') {
                        return -1;
                    }
                } else
                    return -1;
            }
        }
    }

    return number_of_command;
}