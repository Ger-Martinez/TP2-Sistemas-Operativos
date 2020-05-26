// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include "stdio.h"
#include "semaphores.h"

extern uint64_t syscall_create_process(uint64_t, int, uint8_t, uint8_t);
extern uint64_t syscall_exit(uint8_t pid_key);
extern uint32_t syscall_getpid(int pid_key);
extern char* num_to_string(int num);

#define N 10000000    // 10.000.000
//#define N 1000000   // 1 millon
#define SEM_ID 1
#define TOTAL_PAIR_PROCESSES 2
#define NO_SEM 1
#define WITH_SEM 0

long int global;  //shared memory
uint8_t process_left_to_finish;  // only the last process can destroy the semaphore

void slowInc(long int *p, int inc, uint8_t state){
  long int aux = *p;
  if(state == NO_SEM) {
    for(uint64_t i=0; i<100; i++){ /* busy waiting */ }
  }
  aux += inc;
  *p = aux;
}

void my_process_inc(uint8_t pid_key){
  uint64_t i;
  int ret;
  uint32_t this_process_pid = syscall_getpid(pid_key);
  if(this_process_pid == 1) {
    print(STD_ERR, "ERROR in syscall_getpid, in my_process_inc\n");
    syscall_exit(pid_key);
  }

  ret = create_semaphore(1, SEM_ID);
  if (ret == -1){
    print(STD_ERR, "ERROR OPENING SEM, in function my_process_inc\n");
    syscall_exit(pid_key);
  }else if(ret == 1) {
    print(STD_OUTPUT, "my_process_inc tried to connect to an already-created SEM. It doesn't matter\n");
  }
  
  for (i = 0; i < N; i++){
    ret = sem_wait(SEM_ID, this_process_pid);
    if(ret == 2 || ret == 1) {
      print(STD_ERR, "ERROR in sem_wait, in my_process_inc\n");
      syscall_exit(pid_key);
    }
    slowInc(&global, 1, WITH_SEM);
    ret = sem_post(SEM_ID, this_process_pid);
    if(ret == 1) {
      print(STD_ERR, "ERROR in sem_post, in my_process_inc\n");
      syscall_exit(pid_key);
    }
  }
  
  print(STD_OUTPUT, "Final value of global in my_process_inc: ");
  print(STD_OUTPUT, num_to_string(global));
  print(STD_OUTPUT, "\n");

  if(process_left_to_finish == 1)
    destroy_semaphore(SEM_ID, this_process_pid);
  else
    print(STD_OUTPUT, "my_process_inc cannot destroy this SEM as this process isn't the last one to execute\n");

  process_left_to_finish--;
  syscall_exit(pid_key);
}

void my_process_dec(uint8_t pid_key){
  uint64_t i;
  int ret;
  uint32_t this_process_pid = syscall_getpid(pid_key);
  if(this_process_pid == 1) {
    print(STD_ERR, "ERROR in syscall_getpid, in my_process_dec\n");
    syscall_exit(pid_key);
  }

  ret = create_semaphore(1, SEM_ID);
  if (ret == -1){
    print(STD_ERR, "ERROR OPENING SEM, in function my_process_inc\n");
    syscall_exit(pid_key);
  }else if(ret == 1){
    print(STD_OUTPUT, "my_process_dec tried to connect to an already-created SEM. It doesn't matter\n");
  }
  
  for (i = 0; i < N; i++){
    ret = sem_wait(SEM_ID, this_process_pid);
    if(ret == 2 || ret == 1) {
      print(STD_ERR, "ERROR in sem_wait, in my_process_dec\n");
      syscall_exit(pid_key);
    }
    slowInc(&global, -1, WITH_SEM);
    ret = sem_post(SEM_ID, this_process_pid);
    if(ret == 1) {
      print(STD_ERR, "ERROR in sem_post, in my_process_dec\n");
      syscall_exit(pid_key);
    }
  }

  print(STD_OUTPUT, "Final value of global in my_process_dec: ");
  print(STD_OUTPUT, num_to_string(global));
  print(STD_OUTPUT, "\n");

  if(process_left_to_finish == 1)
    destroy_semaphore(SEM_ID, this_process_pid);
  else
    print(STD_OUTPUT, "my_process_dec cannot destroy this SEM as this process isn't the last one to execute\n");

  process_left_to_finish--;
  syscall_exit(pid_key);
}

uint8_t test_sync(uint8_t background, uint8_t pid_key){
  uint64_t i;
  int ret;
  global = 0;
  process_left_to_finish = TOTAL_PAIR_PROCESSES * 2;

  print(STD_OUTPUT, "CREATING PROCESSES...\n");

  void (*p1)(uint8_t);
  void (*p2)(uint8_t);
  p1 = my_process_inc;
  p2 = my_process_dec;

  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){

    ret = syscall_create_process((uint64_t)p1, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
      print(STD_ERR, "ERROR in syscall_create_process, in test_sync\n");
      return 1;
    }

    ret = syscall_create_process((uint64_t)p2, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
      print(STD_ERR, "ERROR in syscall_create_process, in test_sync\n");
      return 1;
    }
  }
  return 0;
  // The last one should print 0
}

///////////////////////////////////////////////////////////

void my_process_inc_no_sem(uint8_t pid_key){
  uint64_t i;
  for (i = 0; i < N; i++){
    slowInc(&global, 1, NO_SEM);
  }

  print(STD_OUTPUT, "Final value of global in my_process_inc_no_sem: ");
  print(STD_OUTPUT, num_to_string(global));
  print(STD_OUTPUT, "\n");

  syscall_exit(pid_key);
}

void my_process_dec_no_sem(uint8_t pid_key){
  uint64_t i;
  for (i = 0; i < N; i++){
    slowInc(&global, -1, NO_SEM);
  }

  print(STD_OUTPUT, "Final value of global in my_process_dec_no_sem: ");
  print(STD_OUTPUT, num_to_string(global));
  print(STD_OUTPUT, "\n");

  syscall_exit(pid_key);
}

uint8_t test_no_sync(uint8_t background, uint8_t pid_key){
  uint64_t i;
  int ret;
  global = 0;

  print(STD_OUTPUT, "CREATING PROCESSES...\n");

  void (*p1)(uint8_t);
  void (*p2)(uint8_t);
  p1 = my_process_inc_no_sem;
  p2 = my_process_dec_no_sem;

  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    ret = syscall_create_process((uint64_t)p1, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
      print(STD_ERR, "ERROR in syscall_create_process, in test_no_sync\n");
      return 1;
    }
    ret = syscall_create_process((uint64_t)p2, background, pid_key, STD_OUTPUT);
    if(ret == 1) {
      print(STD_ERR, "ERROR in syscall_create_process, in test_no_sync\n");
      return 1;
    }
  }
  return 0;
  // The last one should not print 0
}
