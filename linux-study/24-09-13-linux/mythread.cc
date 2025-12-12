#include"Thread.hpp"
#include<unistd.h>
#include<memory>


void* start_routine(void* args)
{
    string name=static_cast<const char*>(args);
    int cnt=5;
    while(cnt--)
    {
        cout<<"我是新线程..."<<name<<endl;
        sleep(1);
    }
}
int main()
{
    unique_ptr<Thread> thread1(new Thread(start_routine,(void*)"thread->1",1));
    unique_ptr<Thread> thread2(new Thread(start_routine,(void*)"thread->2",2));
    unique_ptr<Thread> thread3(new Thread(start_routine,(void*)"thread->3",3));
    unique_ptr<Thread> thread4(new Thread(start_routine,(void*)"thread->4",4));

    thread1->join();
    thread2->join();
    thread3->join();
    thread4->join();


    return 0;
}












// #include<iostream>
// #include<pthread.h>
// #include<unistd.h>
// #include<cstring>

// using namespace std;

// //添加__thread,可以将一个内置属性设置为线程局部存储
// __thread int g_val=0;

// string changeId(const pthread_t &thread_id)
// {
//     char tid[128];
//     snprintf(tid, sizeof(tid), "0x%x", thread_id);
//     return tid;
// }

// void* start_routine(void* args)
// {
//     string threadname=static_cast<const char*>(args);
//     //pthread_detach(pthread_self());//设置自己为分离状态
//     //int cnt=5;
//     while(true)
//     {
//         cout<<threadname<<" running ... : "<<changeId(pthread_self())<<\
//         " g_val: "<<g_val<<" &g_val: "<<&g_val<<endl;
//         ++g_val;
//         sleep(1);
//     }

//     return nullptr;
// }

// int main()
// {

//     pthread_t tid;
//     pthread_create(&tid,nullptr,start_routine,(void*)"thread 1");
//     pthread_detach(tid);//主线程对新线程进行分离
//     string main_id=changeId(pthread_self());
//     while(true)
//     {
//         cout<<"main thread running ... new thread id:"<<changeId(tid)<<" main thread id: "<<main_id<<\
//         " g_val: "<<g_val<<" &g_val: "<<&g_val<<endl;
//         sleep(1);
//     }
    
//     //sleep(2);
//     //一个线程默认是joinable的，如果设置了分离状态，不能够进行等待了
//     // int n=pthread_join(tid,nullptr);
//     // cout<<"result: "<<n<<" : "<<strerror(n)<<endl;
//     return 0;
// }