// 仅用于学习测试：用户进程通过此系统调用竞争同一把内核睡眠读写锁
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "rwsleeplock.h"

static struct rwsleeplock rw;
static int value;
static int check;

void
rwsleeptestinit(void)
{
  rwsleepinit(&rw);
}

uint64
sys_rwsleeptest(void)
{
  int op, state;
  argint(0, &op);
  switch(op) {
  case 0: rwsleep_read_acquire(&rw); return 0;
  case 1: rwsleep_read_release(&rw); return 0;
  case 2: rwsleep_write_acquire(&rw); return 0;
  case 3: rwsleep_write_release(&rw); return 0;
  case 4: // 状态快照：读者、写者、等待读者、等待写者
    acquire(&rw.guard);
    state = rw.readers | (rw.writer << 8) |
            (rw.waiting_readers << 9) | (rw.waiting_writers << 17);
    release(&rw.guard);
    return state;
  case 5: // 在写锁内主动让出 CPU，扩大并发交错的机会
    rwsleep_write_acquire(&rw);
    value++;
    yield();
    check = value * 2;
    rwsleep_write_release(&rw);
    return 0;
  case 6: // 若读写互斥被破坏，可能看到不一致的数据
    rwsleep_read_acquire(&rw);
    state = (check == value * 2);
    yield();
    state = state && (check == value * 2);
    rwsleep_read_release(&rw);
    return state ? 0 : -1;
  case 7:
    rwsleep_read_acquire(&rw);
    state = value;
    rwsleep_read_release(&rw);
    return state;
  case 8:
    rwsleep_write_acquire(&rw);
    value = check = 0;
    rwsleep_write_release(&rw);
    return 0;
  default: return -1;
  }
}
