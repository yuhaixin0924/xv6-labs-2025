#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#define QUEUE_SIZE 4

struct pc_queue {
  int data[QUEUE_SIZE];

  int read_pos;
  int write_pos;
  int count;

  struct spinlock lock;

  char not_empty_channel;
  char not_full_channel;
};

static struct pc_queue q;

void pcqinit(void){
    initlock(&q.lock,"pcqueue");
}
uint64 sys_qput(void) { 
    struct proc * p=myproc();
    int num;
    argint(0,&num);
    acquire(&q.lock);//互斥性：一次只能有一个进程拿到队列的锁，保证互斥
    while(q.count==QUEUE_SIZE){
        if(killed(p)){
            release(&q.lock);
            return -1;
        }
        sleep(&q.not_full_channel,&q.lock);
    }
    q.data[q.write_pos]=num;
    q.write_pos=(q.write_pos+1)%QUEUE_SIZE;
    q.count++;
    wakeup(&q.not_empty_channel);
    release(&q.lock);
    return 0;
}
uint64 sys_qget(void){
    struct proc * p=myproc();
    int value;
    uint64 user_addr;
    argaddr(0, &user_addr);
    acquire(&q.lock);
    while(q.count==0){
        if(killed(p)){
            release(&q.lock);
            return -1;
        }
        sleep(&q.not_empty_channel,&q.lock);
    }
    value=q.data[q.read_pos];
    // 在出队前验证并复制：地址无效时保留队列中的数据。
    if(copyout(p->pagetable, user_addr, (char *)&value, sizeof(value)) < 0){
        release(&q.lock);
        return -1;
    }
    q.read_pos=(q.read_pos+1)%QUEUE_SIZE;
    q.count--;
    wakeup(&q.not_full_channel);
    release(&q.lock);
    return 0;
}
