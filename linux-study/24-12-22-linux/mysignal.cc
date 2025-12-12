#include<iostream>
#include<cstdio>
#include<unistd.h>
#include<sys/types.h>
#include<signal.h>
#include<string>

using namespace std;
void hardler(int signo)
{
    cout<<"进程捕捉到一个信息,信号是："<<signo<<endl;
}

void Usage(const string& proc)
{

    cout<<"\nUsage"<<proc<<"pid signo\n"<<endl;
}

int cnt=0;
void catchSig(int signo)
{
    //cout<<"进程捕捉到一个信息,信号是："<<signo<<<<endl;
    cout<<"进程捕捉到一个信息,信号是："<<cnt<<endl;
    alarm(1);
}


int main(int argc,char* argv[])
{


    while(true)
    {
        int a[10];
        a[10000]=10;
    }
    // alarm(1);
    // signal(SIGALRM,catchSig);
    // while(true)
    // {
    //     cnt++;
    //     //printf("我是一个进程,pid:%d,cnt=%d\n",getpid(),cnt++);

    // }

    
    // signal(SIGFPE,catchSig);

    // while(true)
    // {
    //     cout<<"我正在运行中。。。。"<<endl;
    //     // int a=10;
    //     // a/=0;
    //     int* ptr=nullptr;
    //     *ptr=10;
    // }

    // int cnt=0;
    // while(true)
    // {
    //     printf("我是一个进程,pid:%d,cnt=%d\n",getpid(),cnt++);
    //     sleep(1);
    //     //if(cnt==5) raise(9);
    //     if(cnt==5) abort();

    // }

    // if(argc != 3)
    // {
    //     Usage(argv[0]);
    // }

    // pid_t pid=stoi(argv[1]);
    // int signo=stoi(argv[2]);
    // int n=kill(pid,signo);
    // if(n != 0)
    // {
    //     perror("kill");
    // }


    // int cnt=0;
    // signal(2,hardler);

    // while(true)
    // {
    //     printf("我是一个进程,pid:%d,cnt=%d\n",getpid(),cnt++);
    //     sleep(1);
    // }
    return 0;
}
