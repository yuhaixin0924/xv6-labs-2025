// 睡眠读写锁：使用短暂持有的自旋锁保护状态。
struct rwsleeplock {
  struct spinlock guard;
  int readers;
  int writer;
  int waiting_writers;
  int waiting_readers; // 便于测试时观察等待的读者
  char readers_chan;
  char writers_chan;
};
