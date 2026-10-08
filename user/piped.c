#include "kernel/types.h"
#include "user/user.h"
int main()
{
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid > 0)
    {                // 父进程，负责读
        close(p[1]); // 关闭不⽤的写端
        char buf[8];
        read(p[0], buf, 4);
        printf("parent: received '%s'\n", buf);
        close(p[0]); // ⽤完读端，也关闭
        wait(0);     // 等待⼦进程
        exit(0);
    }
    else
    {                // 子进程，负责写
        close(p[0]); // 关闭不⽤的读端
        write(p[1], "hi~", 4);
        close(p[1]); // ⽤完写端，也关闭
        exit(0);
    }
}