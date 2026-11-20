#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "rwsleeplock.h"

// 使用条件：进程上下文，不持有其他自旋锁或额外的 push_off
void
rwsleepinit(struct rwsleeplock *rw)
{
  initlock(&rw->guard, "rwsleep guard");
  rw->readers = 0;
  rw->writer = 0;
  rw->waiting_writers = 0;
  rw->waiting_readers = 0;
}

void
rwsleep_read_acquire(struct rwsleeplock *rw)
{
  acquire(&rw->guard);
  rw->waiting_readers++;
  while(rw->writer || rw->waiting_writers) {
    
    sleep(&rw->readers_chan, &rw->guard);
    
  }
  rw->waiting_readers--;
  rw->readers++;
  release(&rw->guard);
}

void
rwsleep_read_release(struct rwsleeplock *rw)
{
  acquire(&rw->guard);
  rw->readers--;
  if(rw->readers == 0 && rw->waiting_writers > 0)
    wakeup(&rw->writers_chan);
  release(&rw->guard);
}

void
rwsleep_write_acquire(struct rwsleeplock *rw)
{
  acquire(&rw->guard);
  rw->waiting_writers++;
  while(rw->writer || rw->readers)
    sleep(&rw->writers_chan, &rw->guard);
  rw->waiting_writers--;
  rw->writer = 1;
  release(&rw->guard);
}

void
rwsleep_write_release(struct rwsleeplock *rw)
{
  acquire(&rw->guard);
  rw->writer = 0;
  if(rw->waiting_writers > 0)
    wakeup(&rw->writers_chan);
  else
    wakeup(&rw->readers_chan);
  release(&rw->guard);
}
