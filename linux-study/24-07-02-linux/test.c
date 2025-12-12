#include<stdio.h>
#include<unistd.h>


int main()
{
  //  while(1)
  //  {
  //      printf("hello linux\n");
  //      sleep(1);
  //  }
  //
  //
  // while(1)
  // {
  //     printf("我是一个进程，我的进程id : %d  父进程id ：%d  \n",getpid(),getppid());
  //     sleep(1);
  // }
    pid_t id=fork();
    if(id == 0)
    {
        //子进程
        while(1)
        {
            printf("我是子进程，我的id : %d, 父进程id : %d, ret : %d \n",getpid(),getppid(),id);
            sleep(1);
        }
        
    }
    else if(id > 0)
    {
        //父进程
        while(1)
        {
            printf("我是父进程，我的id : %d, 父进程id : %d, ret : %d \n",getpid(),getppid(),id);
            sleep(3);
        }
    }
    else
    {
        perror("fork fail\n");
        return 1;
    }
    return 0;
}
