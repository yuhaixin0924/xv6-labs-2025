#include "kernel/types.h"
#include "user/user.h"

// 测试专用操作：仅在本测试中成对获取、释放锁。
enum { RDLOCK, RDUNLOCK, WRLOCK, WRUNLOCK, SNAPSHOT, WRITE, READ, VALUE, RESET };

static void
fail(char *message)
{
  printf("rwsleeptest: FAIL: %s\n", message);
  exit(1);
}

static void
wait_state(int mask, int expected)
{
  for(int i = 0; i < 500; i++) {
    if((rwsleeptest(SNAPSHOT) & mask) == expected)
      return;
    pause(1);
  }
  fail("timeout waiting for lock state");
}

static void
sendbyte(int fd)
{
  if(write(fd, "x", 1) != 1) fail("pipe write");
}

static void
recvbyte(int fd)
{
  char c;
  if(read(fd, &c, 1) != 1) fail("pipe read");
}

int
main(void)
{
  int wready[2], wrelease[2], rready[2], r2ready[2], r2release[2];
  int status;
  if(pipe(wready) < 0 || pipe(wrelease) < 0 || pipe(rready) < 0 ||
     pipe(r2ready) < 0 || pipe(r2release) < 0)
    fail("pipe");

  rwsleeptest(RESET);

  // 主进程是第一个读者，先占有读资格。
  rwsleeptest(RDLOCK);
  int r2 = fork();
  if(r2 < 0) fail("fork second active reader");
  if(r2 == 0) {
    rwsleeptest(RDLOCK);
    sendbyte(r2ready[1]);
    recvbyte(r2release[0]);
    rwsleeptest(RDUNLOCK);
    exit(0);
  }
  recvbyte(r2ready[0]);
  wait_state(0xff, 2); // 两个读者确实同时持有读资格。
  int w = fork();
  if(w < 0) fail("fork writer");
  if(w == 0) {
    rwsleeptest(WRLOCK);
    sendbyte(wready[1]);
    recvbyte(wrelease[0]);
    rwsleeptest(WRUNLOCK);
    exit(0);
  }
  // 等到写者登记等待后，才启动第二个读者。
  wait_state(0x7ffe0000, 1 << 17);
  int r = fork();
  if(r < 0) fail("fork reader");
  if(r == 0) {
    rwsleeptest(RDLOCK);
    sendbyte(rready[1]);
    rwsleeptest(RDUNLOCK);
    exit(0);
  }
  // 即使已有读者在读，新读者也必须让等待中的写者先进入。
  wait_state(0x0001fe00, 1 << 9);
  rwsleeptest(RDUNLOCK);
  // 尚有第二个读者，写者必须继续等待。
  wait_state(0x7ffe01ff, (1 << 17) | 1);
  sendbyte(r2release[1]);
  recvbyte(wready[0]);
  wait_state(0x0001ffff, (1 << 8) | (1 << 9));
  sendbyte(wrelease[1]);
  recvbyte(rready[0]);
  if(wait(&status) < 0 || status != 0) fail("first child");
  if(wait(&status) < 0 || status != 0) fail("second child");
  if(wait(&status) < 0 || status != 0) fail("third child");
  printf("rwsleeptest: writer priority and wakeup OK\n");

  // 多进程读写；写者在两次修改之间主动 yield。
  const int workers = 6, loops = 100;
  for(int i = 0; i < workers; i++) {
    int pid = fork();
    if(pid < 0) fail("fork stress worker");
    if(pid == 0) {
      for(int j = 0; j < loops; j++)
        if(rwsleeptest(i < 3 ? WRITE : READ) != 0) exit(1);
      exit(0);
    }
  }
  for(int i = 0; i < workers; i++)
    if(wait(&status) < 0 || status != 0) fail("inconsistent read");
  if(rwsleeptest(VALUE) != 3 * loops) fail("incorrect final value");
  printf("rwsleeptest: concurrent reads/writes OK\n");
  printf("rwsleeptest: PASS\n");
  exit(0);
}
