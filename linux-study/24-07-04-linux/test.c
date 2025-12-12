#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
    //想让工作目录是谁，就把谁的路径传过来
    chdir("/home/wdl");
    while(1)
    {
        printf("我是一个进程，我的id是：%d\n",getpid());
        sleep(1);
    }
     
    return 0;
}
