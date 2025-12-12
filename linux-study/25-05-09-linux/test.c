#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
    
int main()
{

    pid_t id=fork();
    if(id == 0)
    {
       // int *p=NULL;
        //子进程
        int cnt=5;
        while(cnt)
        {
            printf("我是子进程, pid:%d, ppid:%d, cnt = %d\n",getpid(),getppid(),cnt--);
            sleep(1);
            //让子进程立刻退出
           // *p=10;
        }
        exit(1);
    }
    //父进程
   // sleep(10);
    int status=0;
    //轮询
    while(1)
    {
        pid_t ret=waitpid(id,&status,WNOHANG);//非阻塞。子进程没有退出，父进程检测的时候，立即返回
        if(ret == 0)
        {   //waitpid 调用成功 && 子进程没退出
            //子进程没有退出，我的waitpid没有等待失败，仅仅检测到子进程没有退出
            printf("wait done,but child is runing.....\n");
        }
        else if(ret > 0)
        {
            //waitpid 调用成功 && 子进程退出了
            printf("wait success:%d,sig number:%d,child exit code:%d\n",ret,status&0x7F,(status>>8)&0xFF);
            break;
        }
        else
        {
            //调用失败
            printf("waitpid call failed\n");
            break;
        }
        sleep(1);
    }
    
  // // sleep(10);
  //  sleep(2);
  //  int status=0;
  // // pid_t ret=wait(NULL);
  //  pid_t ret=waitpid(id,&status,0);
  //  if(ret>0)
  //  {
  //      //判断是否正常退出
  //      if(WIFEXITED(status))
  //      {
  //          //判断子进程运行结果是否正确
  //          printf("exit code:%d\n",WEXITSTATUS(status));
  //      }
  //      else
  //      {
  //          printf("child exit not normal!\n");
  //      }
  //  }
   // printf("wait success:%d",ret);
   // printf("wait success:%d,ret:%d\n",ret,status);
   // printf("wait success:%d,sig number:%d,child exit code:%d\n",ret,status&0x7F,(status>>8)&0xFF);
    sleep(2);
    return 0;
}

