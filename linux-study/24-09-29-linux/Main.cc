#include"ringqueue.hpp"
#include<ctime>
#include<sys/types.h>
#include<unistd.h>
#include"Task.hpp"

string Selfname()
{
    char name[64];
    snprintf(name,sizeof name,"thread[0x%x]",pthread_self());
    return name;
}

void* productor(void* args)
{
    //ringqueue<int>* rq=static_cast<ringqueue<int>*>(args);
    ringqueue<Task>* rq=static_cast<ringqueue<Task>*>(args);


    while(true)
    {
        //生产活动
        //version1
        // int data=rand()&10+1;
        // rq->push(data);
        // cout<<"生产完成,生产的数据是: "<<data<<endl;
        //sleep(1);//生产慢一点

        //version2
        //构建or获取任务

        int x=rand()%10+1;
        int y=rand()%5;
        char op=oper[rand()%oper.size()];
        Task t(x,y,op,mymath);
        //生产任务
        rq->push(t);
        //输出提示
        //cout<<"生产者派发了一个任务: "<<t.toTaskString()<<endl;
        cout<<Selfname()<<" ,生产者派发了一个任务: "<<t.toTaskString()<<endl;

        sleep(1);

    }
}

void* consumer(void* args)
{
    //ringqueue<int>* rq=static_cast<ringqueue<int>*>(args);
    ringqueue<Task>* rq=static_cast<ringqueue<Task>*>(args);

    while(true)
    {
        //消费活动
        //version1
        // int data;
        // rq->pop(&data);
        // cout<<"消费完成,消费的数据是: "<<data<<endl;
        // sleep(1);//消费慢一点

        //vecrsion2
        Task t;
        //消费任务
        rq->pop(&t);
        //cout<<"消费者消费了一个任务: "<<t()<<endl;
        cout<<Selfname()<<" ,消费者消费了一个任务: "<<t()<<endl;
    }
}

int main()
{
    //随机数种子,这里为了更随机
    srand((unsigned int)time(nullptr)^getpid());
    //ringqueue<int>* rq=new ringqueue<int>();
    ringqueue<Task>* rq=new ringqueue<Task>();

    pthread_t p[4],c[2];
    for(int i=0;i<4;++i)
    {
        pthread_create(p+i,nullptr,productor,rq);
    }
    
    for(int i=0;i<2;++i)
    {
        pthread_create(c+i,nullptr,consumer,rq);
    }


    // pthread_join(p,nullptr);
    // pthread_join(c,nullptr);
   //pthread_join(s,nullptr);

    for(int i=0;i<4;++i)
    {
        pthread_join(p[i],nullptr);
    }
    
    for(int i=0;i<2;++i)
    {
        pthread_join(c[i],nullptr);
        
    }

    // pthread_t p,c;
    // pthread_create(&p,nullptr,productor,rq);
    // pthread_create(&c,nullptr,consumer,rq);

    // pthread_join(p,nullptr);
    // pthread_join(c,nullptr);

    return 0;
}