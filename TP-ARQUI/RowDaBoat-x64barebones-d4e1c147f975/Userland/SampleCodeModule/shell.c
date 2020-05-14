#include "stdio.h"
#include "shellCommands.h"
#include "string.h"
#include <stdint.h>

#define SHELL_BUFFER_SIZE ( (128)+(128/2) )  // 1 linea y media

static void shell_main(uint8_t pid_key);
static void search_command(char command_with_possible_parameter[], char* parameter, uint8_t* background);
static void restart_resources_used(uint8_t* background);
extern char* num_to_string(int num);

static char shell_buffer[SHELL_BUFFER_SIZE] = {'\0'};
static uint8_t k = 0; // to move across the buffer
static int commands_to_execute[2] = {0}; // normally, only one command is run, unless a pipe is provided
static uint8_t number_of_commands_to_execute = 0;

void start_shell(uint8_t pid_key) {
    print(STD_OUTPUT, ">>");
    shell_main(pid_key);
}

static void shell_main(uint8_t pid_key) {
    char c;
    char* parameter = "X";
    uint8_t background = 0;
    while(1) {
        c = getChar(STD_INPUT);
        if(c != -1) {
            if(c == '\n') {
                putchar(STD_OUTPUT, c);     // print the '\n' to move to the next line
                shell_buffer[k++] = '\0';
                
                search_command(shell_buffer, parameter, &background);
                if(number_of_commands_to_execute != 0)
                    execute_command(commands_to_execute, number_of_commands_to_execute, parameter, pid_key, background);
                else
                    print(STD_OUTPUT, "   Command not found");
                putchar(STD_OUTPUT, c);  // print the '\n' to move to the next line

                restart_resources_used(&background);
                parameter = "X";
                print(STD_OUTPUT, ">>");
            }
            else if(c=='\b' && k>0) {
                shell_buffer[k--] = '\0';
                putchar(STD_OUTPUT, c);
            }
            else { // it is a normal character
                if(k < SHELL_BUFFER_SIZE) {
                    shell_buffer[k++] = c;
                    putchar(STD_OUTPUT, c);
                }
            }
        } else
            print(STD_ERR, "Now that the shell blocks when it cannot READ, this text should never be seen\n");
    }
}

static void restart_resources_used(uint8_t* background) {
    // empty the shell buffer
    for(uint8_t i=0; shell_buffer[i] != '\0'; i++) {
        shell_buffer[i] = '\0';
    }
    k = 0;
    *background = 0;
    number_of_commands_to_execute = 0;
}


// places in commands_to_execute[] the command(s) that will be executed (MAX 2).
// if a pipe is used, then both commands must exist. If not, error is returned.
static void search_command(char command_with_possible_parameter[], char* parameter, uint8_t* background) {
    int number_of_command = -1;
    uint8_t i=0;

    // we save the first word typed
    char command_without_parameter[SHELL_BUFFER_SIZE] = {'\0'};
    for( ; command_with_possible_parameter[i]!='\0' && command_with_possible_parameter[i]!=' ' && command_with_possible_parameter[i]!='|'; i++){
        command_without_parameter[i] = command_with_possible_parameter[i];
    }
    command_without_parameter[i] = '\0';

    uint8_t command_found = 0;

    // we check if this first word matches a known command
    for(uint8_t j = 0; j < NUMBER_OF_COMMANDS && !command_found; j++) {
        if( strcmp(command_without_parameter, all_commands[j]) == 0) {
            number_of_command = j;
            command_found = 1;
        } 
    }
    if(!command_found)
        return;
    
    commands_to_execute[0] = number_of_command;
    number_of_commands_to_execute++;

    // if a valid command was found, we have to check if there was a parameter or a '&' next to it
    if(command_with_possible_parameter[i] == ' ') {

        // if there was an '&', then after it we must find NOTHING
        if(command_with_possible_parameter[i+1] == '&') {
            *background = 1;
            i++;
            if(command_with_possible_parameter[i+1] != '\0'){
                number_of_commands_to_execute = 0;
                return;
            }
        } 

        // if there was a parameter, we retrieve it
        else {
            uint8_t j2=0, j1;
            for(j1=i+1; command_with_possible_parameter[j1]!='\0' && command_with_possible_parameter[j1] != ' '; j1++) {
                parameter[j2++] = command_with_possible_parameter[j1];
            }
            parameter[j2++] = '\0';

            // if there was an '&' after the parameter, then after the '&' we must find NOTHING
            if(command_with_possible_parameter[j1] == ' ') {
                if(command_with_possible_parameter[j1+1] == '&'){
                    j1++;
                    *background = 1;
                    if(command_with_possible_parameter[j1+1] != '\0') {
                        number_of_commands_to_execute = 0;
                        return;
                    }
                } else {
                    number_of_commands_to_execute = 0;
                    return;
                }
            }
        }
    } 
    // we also have to check if there was a pipe after the first command
    else if(command_with_possible_parameter[i] == '|') {
        i++;
        uint8_t j;
        for( j = 0; command_with_possible_parameter[i]!='\0'; i++){
            command_without_parameter[j++] = command_with_possible_parameter[i];
        }
        command_without_parameter[j] = '\0';

        for(uint8_t k = 0, command_found = 0; k < NUMBER_OF_COMMANDS && !command_found; k++) {
            if( strcmp(command_without_parameter, all_commands[k]) == 0) {
                number_of_command = k;
                command_found = 1;
            } 
        }
        if(!command_found) {
            number_of_commands_to_execute = 0;
            return;
        }

        commands_to_execute[1] = number_of_command;
        number_of_commands_to_execute++;
    }
}