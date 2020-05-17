#include <stdint.h>
#include "stdio.h"

#define MINOR_WAIT 1000000       // TODO: To prevent a process from flooding the screen
#define WAIT       100000000     // TODO: Long enough to see theese processes beeing run at least twice
#define TOTAL_PROCESSES 3

extern uint64_t syscall_create_process(uint64_t, int, uint8_t, uint8_t);
extern uint64_t syscall_kill(uint32_t PID);
extern char* num_to_string(int number);
extern uint64_t syscall_block(uint32_t PID, uint32_t PID_of_calling_process);
extern uint32_t syscall_getpid(int pid_key);
extern uint64_t syscall_nice(uint8_t pid, uint8_t priority);
extern uint64_t syscall_exit(uint8_t pid_key);
void test_priority(uint8_t pid_key);

// command
void testing_priority(uint8_t background, uint8_t pid_key) {
    void (*p)(uint8_t);
    p = test_priority;
    // we create a process, which will start its execution on the "test_priority" function
    uint32_t ret = syscall_create_process((uint64_t)p, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
        print(STD_ERR, "ERROR: could not create process \"test_priority\"\n");
    }
}

void bussy_wait(uint64_t n){
  uint64_t i;
  for (i = 0; i < n; i++);
}

void endless_loop_for_priority(uint8_t pid_key){
  uint64_t pid = syscall_getpid(pid_key);
  if(pid == 1) {
    print(STD_ERR, "Error in getpid, in function endless_loop_for_priority\n");
    syscall_exit(pid_key);
  }

  while(1){
    print(STD_OUTPUT, num_to_string(pid));
    print(STD_OUTPUT, " ");
    bussy_wait(MINOR_WAIT);
  }
}

// process code
void test_priority(uint8_t pid_key){
  int ret;
  uint32_t this_process_pid = syscall_getpid(pid_key);
  if(this_process_pid == 1) {
    print(STD_ERR, "Error in getpid, in function test_priority\n");
    syscall_exit(pid_key);
  }
  uint64_t pids[TOTAL_PROCESSES];
  uint64_t i;
  void(*foo)(uint8_t);
  foo = endless_loop_for_priority;

  for(i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = syscall_create_process((uint64_t)foo, 1, pid_key, STD_OUTPUT);
    if(pids[i] == 1) {
      print(STD_ERR, "Error: could not create process \"endless_loop_for_priority\"\n");
      syscall_exit(pid_key);
    }
  }

  bussy_wait(WAIT);
  print(STD_OUTPUT, "\nCHANGING PRIORITIES...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++){
    switch (i % 3){
      case 0:
        ret = syscall_nice(pids[i], 1); //lowest priority
        if(ret == 1) {
          print(STD_ERR, "Error in syscall_nice, in test_priority\n");
          syscall_exit(pid_key);
        }
        break;
      case 1:
        ret = syscall_nice(pids[i], 2); //medium priority
        if(ret == 1) {
          print(STD_ERR, "Error in syscall_nice, in test_priority\n");
          syscall_exit(pid_key);
        }
        break;
      case 2:
        ret = syscall_nice(pids[i], 3); //highest priority
        if(ret == 1) {
          print(STD_ERR, "Error in syscall_nice, in test_priority\n");
          syscall_exit(pid_key);
        }
        break;
    }
  }

  bussy_wait(WAIT);
  print(STD_OUTPUT, "\nBLOCKING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++){
    ret = syscall_block(pids[i], this_process_pid);
    if(ret == 1) {
      print(STD_ERR, "Error in syscall_block, in test_priority\n");
      syscall_exit(pid_key);
    }
  }

  print(STD_OUTPUT, "CHANGING PRIORITIES WHILE BLOCKED...\n");
  for(i = 0; i < TOTAL_PROCESSES; i++){
    switch (i % 3){
      case 0:
        ret = syscall_nice(pids[i], 2); //medium priority
        if(ret == 1) {
          print(STD_ERR, "Error in syscall_nice, in test_priority\n");
          syscall_exit(pid_key);
        }
        break;
      case 1:
        ret = syscall_nice(pids[i], 2); //medium priority
        if(ret == 1) {
          print(STD_ERR, "Error in syscall_nice, in test_priority\n");
          syscall_exit(pid_key);
        }
        break;
      case 2:
        ret = syscall_nice(pids[i], 2); //medium priority
        if(ret == 1) {
          print(STD_ERR, "Error in syscall_nice, in test_priority\n");
          syscall_exit(pid_key);
        }
        break;
    }
  }

  print(STD_OUTPUT, "UNBLOCKING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++){
    ret = syscall_block(pids[i], this_process_pid);
    if(ret == 1) {
      print(STD_ERR, "Error in syscall_block, in test_priority\n");
      syscall_exit(pid_key);
    }
  }

  bussy_wait(WAIT);
  print(STD_OUTPUT, "\nKILLING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++){
    ret = syscall_kill(pids[i]);
    if(ret == 1) {
      print(STD_ERR, "Error in syscall_kill, in test_priority\n");
      syscall_exit(pid_key);
    }
  }
  
  syscall_exit(pid_key);
}