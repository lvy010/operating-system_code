#include<stdio.h>
#include<unistd.h>

int global_val=100;

int main()
{
   pid_t id=fork();
   if(id == 0)
   {
       int cnt=0;
       while(1)
       {
           printf("我是子进程, pid: %d, ppid: %d | global_value: %d, &global_value: %p\n", getpid(), getppid(), global_val, &global_val);
           sleep(1);
           cnt++;
           if(cnt == 10)
           {
               global_val=200;
               printf("子进程已经改变global_val的值了。。。。。\n");
           }
       }
   }
   else if(id > 0)
   {
       while(1)
       {
           printf("我是父进程, pid: %d, ppid: %d | global_value: %d, &global_value: %p\n", getpid(), getppid(), global_val, &global_val);
           sleep(2);
           
       }
   }
   else
   {
       printf("fork error\n");
   }



   return 0;

}

//#include <stdio.h>
//#include <unistd.h>
//
//int global_value = 100;
//
//int main()
//{
//    pid_t id = fork();
//    if(id < 0)
//    {
//        printf("fork error\n");
//        return 1;
//    }
//    else if(id == 0)
//    {
//        int cnt = 0;
//        while(1)
//        {
//            printf("我是子进程, pid: %d, ppid: %d | global_value: %d, &global_value: %p\n", getpid(), getppid(), global_value, &global_value);
//            sleep(1);
//            cnt++;
//            if(cnt == 10)
//            {
//                global_value = 300;
//                printf("子进程已经更改了全局的变量啦..........\n");
//            }
//        }
//    }
//    else
//    {
//        while(1)
//        {
//            printf("我是父进程, pid: %d, ppid: %d | global_value: %d, &global_value: %p\n", getpid(), getppid(), global_value, &global_value);
//            sleep(2);
//        }
//    }
//    sleep(1);
//}
