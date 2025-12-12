#include<iostream>
#include<pthread.h>
#include<cassert>
#include<unistd.h>
#include<stdio.h>

using namespace std;

int g_val=0;
//新线程
void* thread(void* arg)
{
    const char *name = (const char *)arg;
    while(true)
    {
        cout << "我是新线程, 我正在运行! name: " << name << " : "<< g_val++ << " &g_val : " << &g_val << endl;
        sleep(1);
    }
}

int main()
{
    pthread_t tid;
    int n=pthread_create(&tid,NULL,thread,(void*)"thread_one");
    assert(n == 0);
    (void)n;

    //主线程
    while(true)
    {
        char tidbuffer[64];
        snprintf(tidbuffer,sizeof tidbuffer,"0x%x",tid);
         cout << "我是主线程, 我正在运行!, 我创建出来的线程的tid： " << tidbuffer << " : " << g_val << " &g_val : " << &g_val << endl;
        sleep(1);
    }

    return 0;

}