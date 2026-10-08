#include "kernel/types.h"
#include "user/user.h"
int main()
{
    int p[2];
    pipe(p);// p[0] 用来读，p[1] 用来写
    int pid = fork();
    if (pid > 0)
    {   
        close(p[0]);
        // ⽗进程
        printf("parent: my child is %d\n", pid);
        write(p[1],"x",1);
        close(p[1]);
        wait(0);
    }
    else if (pid == 0)
    {   
        close(p[1]);
        char msg;
        read(p[0],&msg,1);
        close(p[0]);
        // ⼦进程
        printf("child: my pid is %d\n", getpid());
        exit(0);
    }
    else
    {
        // 错误处理
        printf("fork failed\n");
        exit(1);
    }
    exit(0);
}