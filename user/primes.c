#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime(int p[])
{
    int num;
    int cnt = 0;
    int arr[50];
    while (read(p[0], &num, sizeof(num)) > 0)
        arr[cnt++] = num;
    close(p[0]);

    if (!cnt)
        return;

    printf("prime %d\n", arr[0]);

    int p2[2];
    pipe(p2);

    for (int i = 1; i < cnt; i++)
    {
        if (arr[i] % arr[0] != 0)
        {
            int n = arr[i];
            write(p2[1], &n, sizeof(n));
            // printf("%d: %d\n", getpid(), n);
        }
    }
    close(p2[1]);

    int pid = fork();

    if (pid == 0)
        prime(p2);
    else
        wait(0);

    close(p2[0]);

    return;
}

int main(int argc, char *argv[])
{

    if (argc != 1)
    {
        fprintf(2, "Usage: primes\n");
        exit(1);
    }

    int p[2];
    pipe(p);

    for (int i = 2; i < 35; i++)
    {
        int num = i;
        write(p[1], &num, sizeof(num));
    }

    close(p[1]);
    prime(p);
    close(p[0]);

    exit(0);
}