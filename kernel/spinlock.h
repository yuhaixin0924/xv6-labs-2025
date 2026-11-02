// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
#ifdef LAB_LOCK
  int nts;
  int n;
#endif
};

#ifdef LAB_LOCK
// Reader-writer lock.
struct rwspinlock {
  // Replace this with your implementation.
  struct spinlock l;
  int readers;          // 正在读取的人数
  int writer;           // 是否有写者正在写：0 没有，1 有
  int waiting_writers;  // 正在等待的写者人数
};
#endif

