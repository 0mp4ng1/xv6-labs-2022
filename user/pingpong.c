#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{

    if (argc != 1)
    {
        fprintf(2, "Usage: pingpong\n");
        exit(1);
    }
    int p[2];
    char buf[100];
    pipe(p);

    int pid = fork();
    if (pid == 0)
    {
        write(p[1], "ping", 4);
        printf("%d: received ping\n", getpid());
    }
    else
    {
        wait(0);
        read(p[0], buf, 4);
        printf("%d: received pong\n", getpid());
    }
    exit(0);
}