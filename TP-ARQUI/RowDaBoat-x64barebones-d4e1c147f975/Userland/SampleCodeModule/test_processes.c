#include "stdio.h"
#include "test_util.h"

#define MAX_PROCESSES 18 //Should be around 80% of the the processes handled by the kernel

extern uint64_t syscall_create_process(uint64_t, int, uint8_t);
extern uint64_t syscall_kill(uint32_t PID);
extern uint64_t syscall_block(uint32_t PID, uint16_t PID_of_calling_process);
extern uint16_t syscall_getpid(uint8_t pid_key);
extern uint64_t syscall_exit(uint8_t pid_key);
/////////////
extern char* num_to_string(int num);
//////////////

void endless_loop(){
  while(1);
}

enum State {ERROR, RUNNING, BLOCKED, KILLED};

typedef struct P_rq{
  uint32_t pid;
  enum State state;
}p_rq;

void test_processes(uint8_t pid_key){
  p_rq p_rqs[MAX_PROCESSES];
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;

  void(*foo)(void);
  foo = endless_loop;

  uint16_t this_process_PID = syscall_getpid(pid_key);

  while (1){

    // Create MAX_PROCESSES processes
    for(rq = 0; rq < MAX_PROCESSES; rq++){
      p_rqs[rq].pid = syscall_create_process((uint64_t)foo, 1, pid_key);

      if (p_rqs[rq].pid == 1){
        print(STD_ERR, "Error creating process\n");
        syscall_exit(pid_key);
      }else{
        p_rqs[rq].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0){

      for(rq = 0; rq < MAX_PROCESSES; rq++){
        action = GetUniform(2) % 2;
        //print("action = "); print(num_to_string(action)); print("\n");

        switch(action){
          case 0:{
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED){
              if(syscall_kill(p_rqs[rq].pid) == 1){
                print(STD_ERR, "Error killing process\n");
                syscall_exit(pid_key);
              }
              p_rqs[rq].state = KILLED; 
              alive--;
            }
            break;
          }
          case 1:{
            if (p_rqs[rq].state == RUNNING){
              if(syscall_block(p_rqs[rq].pid, this_process_PID) == 1){
                print(STD_ERR, "Error blocking process\n");
                syscall_exit(pid_key);
              }
              p_rqs[rq].state = BLOCKED;
            }
            break;
          }
        }
      }

      // Randomly unblocks processes
      for(rq = 0; rq < MAX_PROCESSES; rq++){
        if (p_rqs[rq].state == BLOCKED/* && GetUniform(2) % 2*/){
          if(syscall_block(p_rqs[rq].pid, this_process_PID) == 1){
            print(STD_ERR, "Error unblocking process\n");
            syscall_exit(pid_key);
          }
          p_rqs[rq].state = RUNNING;
        }
      }
    }
  }
  print(STD_ERR, "JAMAS\n");
  syscall_exit(pid_key);
}