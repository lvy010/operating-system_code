#include<iostream>
#include<signal.h>
#include<unistd.h>
#include<vector>

using namespace std;

#define BLOCK_SIGNAL 2
#define MAX_SIGNUM 31

vector<int> sigarr={2,3,9};

void show_pending(const sigset_t& pending)
{
    for(int signo=MAX_SIGNUM;signo>=1;signo--)
    {
        if(sigismember(&pending,signo))
        {
            cout<<"1";
        }
        else
        {
            cout<<"0";
        }
    }
    cout<<endl;
}

void handler(int signo)
{
    cout<<signo<<"号信号已被抵达！！"<<endl;
}

int main()
{
    
    for(auto& sig:sigarr) signal(sig,handler);
    //1.先尝试屏蔽指定信号
    sigset_t block,oblock;
    //1.1初始化
    sigemptyset(&block);
    sigemptyset(&oblock);
    //1.2添加要屏蔽的信号
    for(auto& sig:sigarr) sigaddset(&block,sig);
    //1.3开始屏蔽，，设置进内核（进程）
    sigprocmask(SIG_SETMASK,&block,&oblock);

    //2.遍历打印pending信号集
    int cnt=5;
    sigset_t pending;
    while(true)
    {
        //2.1初始化
        sigemptyset(&pending);
        //2.2获取pending信号集
        sigpending(&pending);
        //打印
        show_pending(pending);
        sleep(1);
        if(cnt-- == 0)
        {       
            cout<<"解除对信号的屏蔽，不屏蔽任何信号"<<endl; 
            sigprocmask(SIG_SETMASK,&oblock,&block);
        }
    }

    return 0;
}