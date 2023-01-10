#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

void xargs(char *command, char *param[], int len)
{
    char buf[32 * MAXARG];
    char tmp;
    char *p;
    int ptr = 0;

    p = buf;
    while (read(0, p, 1) > 0)
    {
        tmp = *p;
        if (tmp == '\n' || tmp == ' ')
        {
            *p = '\0';
            param[len++] = &buf[ptr];
            ptr += strlen(param[len - 1]) + 1;
        }
        p++;
    }

    if (fork() == 0)
    {
        exec(command, param);
        exit(0);
    }
    else
        wait(0);
}

int main(int argc, char *argv[])
{
    char *command = "echo";
    char *param[MAXARG];
    int len = 0;

    if (argc > 1)
    {
        command = argv[1];
        for (int i = 1; i < argc; i++)
            param[len++] = argv[i];
    }

    xargs(command, param, len);

    exit(0);
}