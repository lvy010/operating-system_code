#include<iostream>
#include<pthread.h>
#include<vector>
#include<string>
#include<unistd.h>
using namespace std;

int tickets=1000;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
// void* GetTicket(void* args)
// {
//     string name=static_cast<const char*>(args);
//     while (true)
//     {
//         pthread_mutex_lock(&lock);
//         if(tickets > 0)
//         {
//             usleep(1000);
//             cout<<name<<" 正在进行抢票: "<<tickets--<<endl;
//             pthread_mutex_unlock(&lock);
//         }
//         else
//         {
//             pthread_mutex_unlock(&lock);
//             break;
//         }
//         usleep(1000);//模拟出票
//     }
//     return nullptr;
// }
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

void* print(void* args)
{
    string name=static_cast<const char*>(args);
    while(true)
    {
        pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond,&lock);
        cout<<name<<endl;
        pthread_mutex_unlock(&lock);
    }
}

int main()
{
// #define NUM 4
//     vector<pthread_t> tids(NUM);
//     for(int i=0;i<NUM;++i)
//     {
//         char* name=new char[64];
//         snprintf(name,64,"thread-%d",i+1);
//         pthread_create(&tids[i],nullptr,GetTicket,name);
//     }

//     for(auto& id:tids)
//     {
//         pthread_join(id,nullptr);
//     }

    pthread_t p,c;
    pthread_create(&p,nullptr,print,(void*)"我是线程A");
    pthread_create(&c,nullptr,print,(void*)"我是线程B");

    while(true)
    {
        sleep(1);
        pthread_cond_signal(&cond);
        //cout<<"main thread wakeup one thread..."<<endl;
    }

    pthread_join(p,nullptr);
    pthread_join(c,nullptr);


    return 0;
}