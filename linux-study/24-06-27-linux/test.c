#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
int AddtoTarget(int from,int end)
{
    int sum=0;
    for(int i=from;i<=end;++i)
    {
        sum+=i;
    }
   // exit(12);
   _exit(12);
   // return sum;
}

int main()
{
    printf("hello linux");
    sleep(2);
   // exit(1);
   _exit(1);

   // int ret=AddtoTarget(1,100);
   // if(ret == 5050)
   //     return 0;
   // else
   //     return 1;

 //   int cnt=0;
 //   while(1)
 //   {
 //       
 //       pid_t id=fork();
 //       if(id < 0)
 //       {
 //           printf("fork error! cnt = %d\n",cnt);
 //       }
 //       else if(id == 0)
 //       {
 //           //子进程
 //           while(1) sleep(1);
 //       }
 //       //父进程
 //       ++cnt;
 //   }


   // printf("hello linux\n");
  // for(int i=0;i<200;++i)
  // {
  //     printf("%d: %s\n",i,strerror(i));
  // }
  
    //写代码是为了完成某件事情，我们如何得知事情完成的怎么样呢？
    //进程退出码
//    int num=AddtoTarget(1,100);
//    if(num == 5050)
//        return 0;
//    else
//        return 1;
    return 0;
}
