#include<iostream>
#include<signal.h>
#include<unistd.h>
#include<vector>

using namespace std;

void sigcb(int signo)
{
    cout<<signo<<" 号信号被捕捉..."<<endl;
}

int main()
{
    vector<int> v={2,40};
    sigset_t set,oset;
    sigemptyset(&set);
    sigemptyset(&oset);
    for(auto& signo:v)
    {
        sigaddset(&set,signo);
    }
    sigprocmask(SIG_SETMASK,&set,&oset);

    while(1) sleep(1);



    // vector<int> v={1,2,3,4,5,6,19,20};
    // for(auto& e:v)
    // {
    //     signal(e,sigcb);
    // }

    // struct sigaction act,oact;
    // act.sa_handler=sigcb;
    // act.sa_mask;
    // sigemptyset(&act.sa_mask);
    // sigaction(2,&act,&oact);

    //while(1) sleep(1);
    return 0;
}