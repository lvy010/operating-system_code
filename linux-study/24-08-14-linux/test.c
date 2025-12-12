#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

int main()
{
   /* while(1)
    {

        printf("我是一个进程，子进程 : %d, 父进程 : %d\n",getpid(),getppid());
        sleep(1);
    }*/

    pid_t id=fork();
    if(id == 0)
    {
        //子进程
        while(1)
        {
            printf("子进程, pid: %d , ppid: %d  id: %d\n" , getpid(),getppid(),id);
            sleep(1);
        }
    }
    else if(id > 0)
    {
        //父进程
        while(1)
        {

            printf("父进程, pid: %d , ppid: %d  id: %d\n" , getpid(),getppid(),id);
            sleep(2);
        }
    }
    else
    {
        perror("fork");
        return 1;
    }
    return 0;
}
