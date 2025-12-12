#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


int main()
{
    pid_t id=fork();
    assert(id != -1);
    if(id == 0)
    {
        execl("./myexe","myexe",NULL);
       // putenv((char*)"MYENV=11223344");
       // extern char**environ;
       // execle("./myexe","myexe",NULL,environ);
       // char* const envp[]={(char*)"MYENV=11223344",NULL};
        // execle("./myexe","myexe",NULL,envp);//自定义环境变量
        //子进程
       // execl("/usr/bin/ls","ls","-l","--color=auto",NULL);
       // 这里有两个ls，其实并不重复，一个是告诉系统我要执行谁，一个是告诉系统，我想怎么执行
       // execlp("ls","ls","-l","--color=auto",NULL);
       // char* const argv[]={"ls","-l","--color=auto",NULL};
       // execv("/usr/bin/ls",argv);
        //  execvp("ls",argv);
        //执行自己写的程序,这里第二个参数可以不带./
       // execl("./myexe","myexe",NULL);
       // execl("./mybin","mybin",NULL);
        exit(1);
    }
    //父进程
    int status=0;
    pid_t ret=waitpid(id,&status,0);
    if(ret > 0)
    {
        printf("wait success:%d,sig number:%d,child exit code:%d\n",ret,status&0x7F,(status>>8)&0xFF);
    }


    
   // printf("process is runing\n");
   // //所有exec系列接口，都必须在传参结束的时候，以NULL结尾
   // execl("/usr/bin/ls","ls","-l","--color=auto",NULL);
   // printf("hello\n");

    return 0;
}



