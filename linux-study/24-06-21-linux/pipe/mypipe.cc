#include<iostream>
#include<unistd.h>
#include<cassert> //C++中使用C的库函数
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<sys/types.h>
#include<sys/wait.h>

using namespace std;

int main()
{
    //父进程创建管道,打开读写端
    int pfd[2];
    int n=pipe(pfd);
    //成功返回0
    assert(n == 0);
    (void)n;//linux默认是release，assert就注释掉了，因此回报没有使用n的警告，这里是消除警告

    //创建子进程
    pid_t fd=fork();
    assert(fd >= 0);
    if(fd == 0)
    {
        //关闭子进程读
        close(pfd[0]);
        //子进程通信代码
        const char* s="我是子进程,我正在给你发信息";
        int cnt=0;
        while(true)
        {
            cnt++;
            char buffer[1024];
            snprintf(buffer,sizeof buffer,"child->parent say: %s[%d][%d]",s,cnt,getpid());
            //写
            write(pfd[1],buffer,strlen(buffer));
            cout<<"count: "<<cnt<<endl;
            //sleep(10); //这里是一个细节,z子进程sleep
            //break;
        }

        //子进程退出
        close(pfd[1]);//子进程关闭写端
        cout<<"子进程关闭了写端"<<endl;
        exit(0);

    }


    //走到这里是父进程
    close(pfd[1]);//关闭父进程写
    //父进程通信代码
    while(true)
    {
        //sleep(1000);
        char buffer[1024];
        //cout<<"AAAAAAAAAAA"<<endl;
        ssize_t s=read(pfd[0],buffer,sizeof(buffer)-1); //我们期望读一个字符串，因此保留一个位置放/0
        //cout<<"BBBBBBBBBBB"<<endl;
        if(s>0) 
        {
            buffer[s]=0;
            cout << "Get Message# " << buffer << " | my pid: " << getpid() << endl;
            //细节父进程可没有sleep
        }
        else if(s == 0)
        {
            //文件读到末尾
            cout<<"read :"<<s<<endl;
            break;
        }
        break;
        
    }

    close(pfd[0]);
    cout<<"父进程关闭了读端"<<endl;

    //回收子进程资源
    int status=0;
    pid_t id=waitpid(fd,&status,0);
    assert(id == fd);
    cout<<"pid->"<<id<<":"<<(status & 0x7F)<<endl;
    // pid_t id=waitpid(fd,nullptr,0);
    // assert(id == fd);

    return 0;
}