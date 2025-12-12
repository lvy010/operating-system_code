#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>

#define NUM 1024
#define OPT_NUM 64

int lastCode=0;
int lastsig=0;

char lineCommand[NUM];
char* myargv[OPT_NUM];//指针数组，存放分割的一个个字符串

int main()
{
    while(1)
    {
         //这里也可以使用环境变量来获取这些内容
         printf("用户名@主机名 当前路径# "); 
         fflush(stdout);
         //"ls -a -l"-----> "ls" "-a" "-l"
         //这里减1，是为了极端情况下放\0
         char* s=fgets(lineCommand,sizeof(lineCommand)-1,stdin);
         assert(s);
         //清除最后一个\n  abc\n
         lineCommand[strlen(lineCommand)-1]=0;
         //测试一下
         //printf("test:%s\n",lineCommand);
         //这里以空格为分隔符
         myargv[0]=strtok(lineCommand," ");
         //分割的是同一个字符串，下一次第一个参数就置为NULL就可以了
         // myargv[1]=strtok(NULL,"");
         //这里需要实现循环，注意strtok到字符串结束，会返回NULL,myargv[end]=NULL
         int i=1;
         //这里对ls，特殊处理
         if(myargv[0] != NULL && strcmp(myargv[0],"ls") == 0)
         {
             myargv[i++]=(char*)"--color=auto";
         }
        
         while(myargv[i++]=strtok(NULL," "));
        //如果是cd命令，不需要创建子进程,让shell自己执行对应的命令，本质就是执行系统接口
        //像这种不需要让我们的子进程来执行，而是让shell自己执行的命令 --- 内建/内置命令
         if(myargv[0] != NULL && strcmp(myargv[0],"cd") == 0)
         {
             if(myargv[1] != NULL)
             {
                 chdir(myargv[1]);
                 continue;
             }
         }

         if(myargv[0] != NULL && strcmp(myargv[0],"echo") == 0)
         {
            if(strcmp(myargv[1],"$?") == 0)
            {
                printf("%d,%d\n",lastCode,lastsig);
            }
            else
            {
                printf("%s\n",myargv[1]);
            }
            continue;
         }
         
//测试分割是否成功
#ifdef DEBUG
         for(int i=0;myargv[i];++i)
         {
             printf("myargv[%d]:%s\n",i,myargv[i]);
         }
#endif

         //创建子进程
         pid_t id = fork();
         assert(id != -1);
         if(id == 0)
         {
             //子进程
             execvp(myargv[0],myargv);
             exit(1);
         }
         //父进程
         int status=0;
         pid_t ret= waitpid(id,NULL,0);
         assert(ret>0);
         lastCode=(status>>8)&0xFF;
         lastsig=status&0x7F;
         return 0;
    }
}
