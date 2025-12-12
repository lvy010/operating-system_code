#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main()
{

   // int a=0;
   // while(1)
   // {
   //     printf("%d\n",a++);
   //     sleep(1);
   // }
   
   // pid_t id=fork();
   // if(id == 0)
   // {
   //     //子进程
   //     int cnt=10;
   //     while(cnt--)
   //     {
   //         printf("子进程, pid : %d , ppid : %d  ,ret : %d  cnt=%d\n",getpid(),getppid(),id,cnt);
   //         sleep(1);
   //     }
   //     exit(-1);
   // }
   // else if(id > 0)
   // {

   //     //父进程
   //     while(1)
   //     {
   //         printf("父进程, pid : %d , ppid : %d  ,ret : %d\n",getpid(),getppid(),id);
   //         sleep(3);
   //     }
   // }
   
    pid_t id=fork();
    if(id > 0)
    {
        //父进程
        int cnt=10;
        while(cnt--)
        {
            printf("父进程, pid : %d , ppid : %d  ,ret : %d  cnt=%d\n",getpid(),getppid(),id,cnt);
            sleep(1);
        }
        exit(-1);
    }
    else if(id == 0)
    {

        //子进程
        while(1)
        {
            printf("子进程, pid : %d , ppid : %d  ,ret : %d\n",getpid(),getppid(),id);
            sleep(3);
        }
    }
    return 0;
}
