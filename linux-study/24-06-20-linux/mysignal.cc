#include<iostream>
#include<unistd.h>
#include<cstdio>
#include<signal.h>
#include<sys/types.h>
#include<string>

using namespace std;

void handler(int signo)
{
    cout<<"进程捕捉到了一个信号，信号编号是:"<<signo<<endl;
    //exit(0);
}

void Usage(const string& proc)
{
    cout<<"\nUsage "<<proc<<" pid signo\n"<<endl;
    sleep(5);
}

int cnt=0;

void catchSig(int signo)
{
    cout<<"进程捕捉到了一个信号，信号编号是:"<<signo<<endl;
    //cout<<"进程捕捉到了一个信号，信号编号是:"<<signo<<" cnt :"<<cnt<<endl;
    //alarm(1);
}

//./mysignal pid signo
int main(int argc,char* argv[])
{
    for(int signo=1;signo<=31;++signo)
    {
        signal(signo,catchSig);
    }

    while(true) sleep(1);


    //核心转储
    // while(true)
    // {
    //     int a[10];
    //     //a[10000]=10;
    // }

    // signal(SIGALRM,catchSig);
    // alarm(5);
    // while(true)
    // {
    //     cnt++;
    //     if(cnt == 3)
    //     {
    //     	int n=alarm(0);
    //     	cout<<n<<endl;
    //     }
    //     sleep(1);
    //     //printf("cnt: %d\n",cnt++);
    // }



    // signal(SIGFPE,handler);
    // signal(11,handler);
    // int cnt=0; 
    // while(true)
    // {
    //     printf("cnt:%d,pid:%d\n",cnt++,getpid());
    //     // int a=10;
    //     // a/=0;  
    //     int* ptr=NULL;
    //     *ptr=10;
    // }

    // int cnt=0;
    // while(true)
    // {
    //     printf("cnt:%d,pid:%d\n",cnt++,getpid());
    //     if(cnt == 10)
    //         abort();
    //     // if(cnt == 10)
    //     //     raise(9);
    //     sleep(1);
    // }

    // if(argc != 3)
    // {
    //     Usage(argv[0]);
    //     exit(1);
    // }
    
    // pid_t id=stoi(argv[1]);
    // int signo=stoi(argv[2]);
    // int n=kill(id,signo);
    // if(n != 0)
    // {
    //     perror("kill");
    // }

//     signal(2,handler);
//     int cnt=0;
//     while(true)
//     {
//         printf("我是一个进程，我正在运行%d\n",cnt++);
//         sleep(1);
//     }
    return 0;
}