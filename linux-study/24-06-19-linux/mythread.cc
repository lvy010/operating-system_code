#include<iostream>
#include<thread>
#include<unistd.h>

using namespace std;

void thread_run()
{
    while(true)
    {
        cout<<"我是新线程..."<<endl;
        sleep(1);
    }
}

int main()
{
    //创建,并执行对应的方法
    thread t1(thread_run);

    while(true)
    {
        cout<<"我是主线程..."<<endl;
        sleep(1);
    }

    //回收
    t1.join();
    return 0;
}







// #include<iostream>
// #include<pthread.h>
// #include<string>
// #include<unistd.h>
// #include<cstdio>
// #include<vector>
// #include<cstdlib>
// #include<cassert>

// using namespace std;





// struct ThreadDate
// {
//     int number;
//     pthread_t tid;
//     char namebuffer[64];
// };

// struct ThreadReturn
// {
//     int exit_code;
//     int exit_result;
// };

// void* start_rountine(void* args)
// {
//     //安全的进行强制类型转化
//     ThreadDate* td=static_cast<ThreadDate*>(args);
//     int cnt=5;
//     while(cnt)
//     {
//         cout<<"cnt: "<<cnt<<" &cnt"<<&cnt<<endl;
//         cnt--;
//         sleep(1);
//         //pthread_exit(nullptr);
//         //exit(1);
//         // cout<<"new thread create success, name: "<<td->namebuffer<<" cnt: "<<cnt-- <<endl;
//         // sleep(1);
//         // int* p=nullptr;
//         // *p=0;
//     }

//     //正常跑完
//     return (void*)100;


//     //注意千万不能直接申请一个对象
//     //ThreadReturn tr; //这是在栈上开辟空间，出了栈就销毁了
//     //tr.exit_code
//     //tr.exit_result
//     //return (void*)&tr

//     // ThreadReturn *tr=new ThreadReturn();//堆开辟的空间
//     // tr->exit_code=1;
//     // tr->exit_result=111;
//     // return (void*)tr;

//     //pthread_exit((void*)111);
//     //return (void*)111;
//     //pthread_exit(nullptr);

//     //return nullptr;
//     //return (void*)td->number;
// }

// // void* start_rountine(void* args)
// // {
// //     //安全的进行强制类型转化
// //     string name=static_cast<const char*>(args);
// //     while(true)
// //     {
// //         cout<<"new thread create success, name: "<<name<<endl;
// //         sleep(1);
// //         // int* p=nullptr;
// //         // *p=0;
// //     }
// // }

// int main()  
// {
//     vector<ThreadDate*> threads;
// #define NUM 10
//     for(int i=0;i<NUM;++i)
//     {

//         ThreadDate* td=new ThreadDate();
//         td->number=i+1;
//         snprintf(td->namebuffer,sizeof(td->namebuffer),"%s:%d","thread",i+1);
//         pthread_create(&td->tid,nullptr,start_rountine,td); 
//         //这样不仅每个线程数据除了自己拿到了，主线程也全部拿到了
//         threads.push_back(td);


//         // pthread_t id;
//         // //pthread_create(&id,nullptr,start_rountine,(void*)"thread new"); 
//         // char namebuffer[64];  
//         // snprintf(namebuffer,sizeof(namebuffer),"%s:%d","thread",i);
//         // pthread_create(&id,nullptr,start_rountine,namebuffer); 
//         // //sleep(1);
//     }


//     for(auto& iter:threads)
//     {
//         cout<<"create thread: "<<iter->namebuffer<<" : "<<iter->tid<<" success"<<endl;
//     }

//     //线程取消
//     sleep(5);//先让线程跑起来
//     for(int i=0;i<threads.size()/2;++i)
//     {
//         pthread_cancel(threads[i]->tid);
//         cout<<"pthread_cancel: "<<threads[i]->namebuffer<<" success"<<endl;
//     }

//     for(auto& iter:threads)
//     {
//         //ThreadReturn* tr=nullptr;
//         void* ret=nullptr;//注意是void*
//         //阻塞式的等待每一个线程
//         int n=pthread_join(iter->tid,&ret);//&地址是void**  
//         //int n=pthread_join(iter->tid,(void**)&tr);//&地址是void**    
//         assert(n == 0);
//         (void)n;
//         //cout<<"join : "<<iter->namebuffer<<" success , number: "<<(int)ret<<endl;
//         cout<<"join : "<<iter->namebuffer<<" success , number: "<<(long long)ret<<endl;
//         //cout<<"join : "<<iter->namebuffer<<" success , exit_code: "<<tr->exit_code<<" exit_result: "<<tr->exit_result<<endl;


//         delete iter;
//     }
//     //这里就可以看出主线程式阻塞式的等待，全部等待成功，才打印这句话
//     cout<<"main thread quit!!"<<endl;
    


//     // while(true)
//     // {
//     //     cout<<"new thread create success, name: main thread"<<endl;
//     //     sleep(1);
//     // }

//     return 0;
// } 