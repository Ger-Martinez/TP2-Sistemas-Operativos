#include <stdint.h>
#include "stdio.h"
#include "semaphores.h"

extern uint64_t syscall_create_process(uint64_t, int, uint8_t, uint8_t);
extern uint64_t syscall_exit(uint8_t pid_key);
extern uint32_t syscall_getpid(int pid_key);
extern char* num_to_string(int num);

#define N 10000000
#define SEM_ID 1
#define TOTAL_PAIR_PROCESSES 2 /*******************************************/

uint64_t global_used_by_synchro;  //shared memory
uint64_t global_used_by_no_synchro;  //shared memory

void slowInc(uint64_t *p, uint64_t inc){
  uint64_t aux = *p;
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
  } else if(ret == 1)
    print(STD_OUTPUT, "my_process_inc tried to connect to an already-created SEM. It doesn't matter\n");
  
  for (i = 0; i < N; i++){
    sem_wait(SEM_ID, this_process_pid);
    slowInc(&global_used_by_synchro, 1);
    sem_post(SEM_ID, this_process_pid);
  }

  destroy_semaphore(SEM_ID, this_process_pid);
  
  print(STD_OUTPUT, "Final value of global_used_by_synchro in my_process_inc: ");
  print(STD_OUTPUT, num_to_string(global_used_by_synchro));
  print(STD_OUTPUT, "\n");

  if(global_used_by_synchro == 0)
    destroy_semaphore(SEM_ID, this_process_pid);
  else
    print(STD_OUTPUT, "my_process_inc cannot destroy this SEM as this process isn't the last one to execute");

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
  } else if(ret == 1)
    print(STD_OUTPUT, "my_process_dec tried to connect to an already-created SEM. It doesn't matter\n");
  
  for (i = 0; i < N; i++){
    sem_wait(SEM_ID, this_process_pid);
    slowInc(&global_used_by_synchro, -1);
    sem_post(SEM_ID, this_process_pid);
  }

  print(STD_OUTPUT, "Final value of global_used_by_synchro in my_process_dec: ");
  print(STD_OUTPUT, num_to_string(global_used_by_synchro));
  print(STD_OUTPUT, "\n");

  if(global_used_by_synchro == 0)
    destroy_semaphore(SEM_ID, this_process_pid);
  else
    print(STD_OUTPUT, "my_process_dec cannot destroy this SEM as this process isn't the last one to execute\n");

  syscall_exit(pid_key);
}

uint8_t test_sync(uint8_t background, uint8_t pid_key){
  uint64_t i;
  int ret;

  global_used_by_synchro = 0;

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
    slowInc(&global_used_by_no_synchro, 1);
  }

  print(STD_OUTPUT, "Final value of global_used_by_no_synchro in my_process_inc_no_sem: ");
  print(STD_OUTPUT, num_to_string(global_used_by_no_synchro));
  print(STD_OUTPUT, "\n");

  syscall_exit(pid_key);
}

void my_process_dec_no_sem(uint8_t pid_key){
  uint64_t i;
  for (i = 0; i < N; i++){
    slowInc(&global_used_by_no_synchro, -1);
  }

  print(STD_OUTPUT, "Final value of global_used_by_no_synchro in my_process_dec_no_sem: ");
  print(STD_OUTPUT, num_to_string(global_used_by_no_synchro));
  print(STD_OUTPUT, "\n");

  syscall_exit(pid_key);
}

uint8_t test_no_sync(uint8_t background, uint8_t pid_key){
  uint64_t i;
  int ret;
  global_used_by_no_synchro = 0;

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
