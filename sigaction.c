#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

void do_sigchile(int num)
{
    printf("信号回收%d\n",num);
}

int main()
{
    struct sigaction newact;
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);
    newact.sa_handler = do_sigchile;
    sigaction(SIGINT,&newact,NULL);
    while(1);
    return 0;
}

