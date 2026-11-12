#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  for(int i = 0; i < 3; i++){
    int pid = fork();

    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pid == 0){
      while(1){
        printf("child %d\n", getpid());
        pause(20);
      }
    }
  }

  for(int i = 0; i < 3; i++)
    wait(0);

  exit(0);
}