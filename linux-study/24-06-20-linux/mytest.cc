#include<iostream>
#include<unistd.h>

int main()
{
    int cnt=0;
    while(true)
    {
        printf("我是一个进程,pid:%d,我正在运行%d\n",getpid(),cnt++);
        sleep(1);
    }
    return 0;
}