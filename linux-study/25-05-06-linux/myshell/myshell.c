#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<string.h>
#include<ctype.h>

#define NUM 1024
#define OPT_NUM 64

//重定向类型
//第一个为初始重定向 
#define DEFAULT_REDIR 0
#define INPUT_REDIR 1
#define OUTPUT_REDIR 2
#define APPEND_REDIR 3

//重定向类型+文件名
int redirType=DEFAULT_REDIR;
char* redirFile=NULL;

int lastCode=0;
int lastsig=0;

char lineCommand[NUM];
char* myargv[OPT_NUM];//指针数组，存放分割的一个个字符串

//这里找文件名没有写成函数，而写个宏
#define trimSpace(start) do{\
    while(isspace(*start)) ++start;\
}while(0)

void commandstrtok(char* cmd)
{
    assert(cmd);
    char* start=cmd;
    char* end=cmd+strlen(cmd);
    while(start < end)
    {
        if(*start == '<')
        {
            *start=0;
            ++start;
            //这里可能 ls -a -l >      log.txt,
            trimSpace(start);
            redirType=INPUT_REDIR;
            redirFile=start;
            break;
        }
        else if(*start == '>')
        {
            *start=0;
            ++start;
            if(*start == '>')
            {
                redirType=APPEND_REDIR;
                ++start;
            }
            else
            {
                redirType=OUTPUT_REDIR;
            }
            trimSpace(start);
            redirFile=start;
            break;
        }
        else
        {
            ++start;
        }
    }

}

int main()
{
   while(1)
   {
       //防止输入ls -a -l > log.txt,在输入ls -a -l还大概在重定向来执行
         redirType=DEFAULT_REDIR;
         redirFile=NULL;
         //这里也可以使用环境变量来获取这些内容
         printf("用户名@主机名 当前路径# "); 
         fflush(stdout);
         //"ls -a -l"-----> "ls" "-a" "-l"
         //这里减1，是为了极端情况下放\0
         char* s=fgets(lineCommand,sizeof(lineCommand)-1,stdin);
         assert(s);

        //增加重定向功能
        //"ls -a -l > log.txt"      "ls -a -l"  "log.txt"
        //"ls -a -l >> log.txt"     "ls -a -l"  "log.txt"
        //"cat < log.txt"           "cat"       "log.txt"
        //分割指令和文件名的函数
        commandstrtok(lineCommand);

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
             
             switch(redirType)
             {
                 case DEFAULT_REDIR:
                     //什么都不做
                     break;
                 case INPUT_REDIR:
                     {
                         int fd=open("log.txt",O_RDONLY);
                         if(fd <0)
                         {
                             perror("open");
                             return 1;
                         }
                         //重定向文件已经打开了
                         dup2(fd,0);
                     }
                     break;
                 case OUTPUT_REDIR:
                 case APPEND_REDIR:
                     {
                         umask(0);
                         int flags=O_WRONLY|O_CREAT;
                         if(redirType == OUTPUT_REDIR) flags|=O_TRUNC;
                         else flags|=O_APPEND;
                         int fd=open("log.txt",flags,0666);
                         if(fd < 0)
                         {
                             perror("open");
                             return 1;
                         }
                         dup2(fd,1);
                     }
                     break;
                 default:
                    printf("bug?\n");
                    break;

             }
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
   }
         return 0;
}
