#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // Your code here.
  #define N 32
  char * hint= "This may help.";
  char * base=sbrk(N*PGSIZE);
  char * page;
  for(int i=0;i<N;i++){
      page=base+i*PGSIZE;
      if(strcmp(page+0x10,hint)==0){
        char * secret=page+0x20;
        printf(secret);
        exit(0);
      }
  }
  exit(1);
}
