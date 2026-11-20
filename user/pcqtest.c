#include "kernel/types.h"
#include "user/user.h"

// 任意一项检查失败时打印原因并结束程序。
static void
fail(char *message)
{
  printf("pcqtest: FAIL: %s\n", message);
  exit(1);
}

int
main(void)
{
  int value, pid, status;

  // 本测试要求开始时共享队列为空；正常完成后也会把队列清空。
  // qget通过指针返回产品，函数返回值只表示成功或失败，
  // 因此产品本身可以是-1。
  printf("[1/5] Checking negative value...\n");
  if(qput(-1) != 0 || qget(&value) != 0 || value != -1)
    fail("negative value");

  // 故意传入无效用户地址：qget应返回-1，而且不能丢掉队首的42。
  printf("[2/5] Checking invalid address...\n");
  if(qput(42) != 0)
    fail("put before invalid address");
  if(qget((int *)0xffffffffffffffffUL) != -1)
    fail("invalid address accepted");
  if(qget(&value) != 0 || value != 42)
    fail("item lost after invalid address");

  // 队列容量为4，反复放取12次，使读写位置多次绕回开头。
  printf("[3/5] Checking queue wraparound...\n");
  for(int i = 0; i < 12; i++){
    if(qput(i) != 0 || qget(&value) != 0 || value != i)
      fail("ring wraparound");
  }

  // 子进程生产0~19，父进程按顺序消费；二者共享内核中的队列。
  printf("[4/5] Checking producer and consumer...\n");
  pid = fork();
  if(pid < 0)
    fail("fork producer");
  if(pid == 0){
    for(int i = 0; i < 20; i++){
      if(qput(i) != 0)
        exit(1);
    }
    exit(0);
  }

  // 给生产者先运行的机会；这有利于观察队列满时的等待，
  // 但单靠延迟不保证一定发生某种调度顺序。
  pause(5);
  for(int i = 0; i < 20; i++){
    if(qget(&value) != 0 || value != i)
      fail("producer-consumer order");
  }
  if(wait(&status) != pid || status != 0)
    fail("producer exit");

  // 队列为空时，让子进程尝试qget；父进程随后终止并回收它。
  // qget的等待循环需要检查killed状态，避免被唤醒后再次无休止等待。
  printf("[5/5] Checking termination of waiting consumer...\n");
  pid = fork();
  if(pid < 0)
    fail("fork waiting consumer");
  if(pid == 0){
    qget(&value);
    exit(0);
  }
  pause(5);
  if(kill(pid) < 0 || wait(&status) != pid)
    fail("kill waiting consumer");

  printf("pcqtest: PASS\n");
  exit(0);
}
