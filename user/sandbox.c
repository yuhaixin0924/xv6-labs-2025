#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/syscall.h"
#include "user/user.h"

void usage(char *s) {
  fprintf(2, "Usage: %s <mask> <path> <command>\n", s);
  exit(1);
}

// Sandbox a command by disallowing system calls in mask and
// system calls that are using path
int
main(int argc, char *argv[])
{
  int i;
  int n = 2;//先跳过程序名和mask字符串
  int mask = 1;
  char *nargv[MAXARG];

  if(argc < 4) {//至少需要程序名、mask、path 和命令这 4 项。
    usage(argv[0]);//参数不足时打印用法并退出。
  }

  if(argv[mask][0] < '0' || argv[mask][0] > '9'){//检查 mask 字符串的第一个字符是否是数字。这段提供的程序只做这一项初步格式检查。
    usage(argv[0]);
  }

  n += 1; // skip path
    
  // strip off the first n arguments to sandbox
  for(i = n; i < argc && i < MAXARG; i++){
    nargv[i-n] = argv[i];//复制字符串地址，例如 nargv[0]=argv[3] 指向 cat，nargv[1]=argv[4] 指向 README
  }
  nargv[argc-n] = 0;//在命令参数数组最后写入空指针，供 exec 判断数组结束。

  int pid = fork();
  if(pid < 0) {
    printf("%s: exec fork failed\n", argv[0]);
    exit(1);
  }
  if(pid == 0) {
    if (interpose(atoi(argv[mask]), argv[mask+1]) < 0) {
      printf("%s: interpose failed", argv[0]);
      exit(1);
    }
    exec(nargv[0], nargv);//在同一个子进程内换成 cat 程序，参数数组为 cat、README、空指针。成功的 exec 不返回这里；进程记录中的 mask 保留。
    printf("%s: exec %s failed\n", argv[0], nargv[0]);//只有 exec 失败才继续到此，打印错误。
    exit(1);
  } else {
    wait(0);//父进程等待一个子进程退出；0 表示不接收其退出状态。
  }
  
  return 0;
}
