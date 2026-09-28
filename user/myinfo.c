#include"kernel/types.h"
#include "user/user.h"
int main(){
    for(int i=0;i<6;i++){
    printf("This is a user program.\n");
    }
    uint64 tick=uptime();
    printf("%lu\n",tick);
    int id=getpid();
    printf("%d\n",id);
    exit(0);
}
