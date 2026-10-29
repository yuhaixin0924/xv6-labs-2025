#include "kernel/types.h"
#include "user/user.h"
int main(int argc, char *argv[])
{
    int pid = fork();
    if (pid == 0)
    {
        set_yield(atoi(argv[1]));
        for (int i = 0; i < 10; i++)
        {
            for (volatile int j = 0; j < 10000000; j++)
            {
            }

            printf("%c[%d]\n", 'C', i);
        }
        exit(0);
    }
    else
    {
        set_yield(atoi(argv[2]));
        for (int i = 0; i < 10; i++)
        {
            for (volatile int j = 0; j < 10000000; j++)
            {
            }

            printf("%c[%d]\n", 'P', i);
        }
        wait(0);
        exit(0);
    }
    exit(0);
}
