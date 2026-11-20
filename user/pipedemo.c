#include "kernel/types.h"
#include "user/user.h"

int main()
{
    int fds[2];
    char ch;
    // fds[0]是读端，fds[1]是写端
    if (pipe(fds) < 0)
    {
        printf("pipe failed\n");
        exit(1);
    }
    int pid = fork();
    if (pid < 0)
    {
        printf("fork failed\n");
        exit(1);
    }
    if (pid == 0)
    {
        close(fds[1]);
        printf("child %d: about to read\n", getpid());
        if (read(fds[0], &ch, 1) != 1)
        {
            printf("child: read failed\n");
            exit(1);
        }

        printf("child %d: received %c\n", getpid(), ch);
        close(fds[0]);
        exit(0);
    }
    else
    {
        close(fds[0]);

        printf("parent %d: pausing before write\n", getpid());
        pause(100);

        printf("parent %d: writing A\n", getpid());
        if (write(fds[1], "A", 1) != 1)
        {
            printf("parent: write failed\n");
            exit(1);
        }

        close(fds[1]);
        wait(0);
        exit(0);
    }
}