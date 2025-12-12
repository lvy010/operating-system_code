#include"Blockqueue.hpp"
#include<ctime>
#include<cstdlib>
#include<unistd.h>
#include"Task.hpp"

template<class C,class S>
class BlockQueues
{
public:
    BlockQueue<C>* c_bq;
    BlockQueue<S>* s_bq;
};


void* producer(void* args)
{
    //BlockQueue<int>* p_bq=static_cast<BlockQueue<int>*>(args);
    //BlockQueue<Task>* p_bq=static_cast<BlockQueue<Task>*>(args);
    BlockQueue<CallTask>* p_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->c_bq;

    while(true)
    {
        int x=rand()%100+1;
        int y=rand()%10;
        int op=rand()%str.size();
        CallTask t(x,y,str[op],mymath);
        p_bq->push(t);
        cout<<"productor thread, 生产计算任务: "<<t.toTaskString()<<endl; 
        // int x=rand()%100;
        // p_bq->push(x);
        // cout<<"生产者任务完成..."<<x<<endl; 
        sleep(1);
    }
    return nullptr;
}

void* consumer(void* args)
{
    //BlockQueue<int>* c_bq=static_cast<BlockQueue<int>*>(args);
    //BlockQueue<Task>* c_bq=static_cast<BlockQueue<Task>*>(args);
    BlockQueue<CallTask>* p_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->c_bq;
    BlockQueue<SaveTask>* c_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->s_bq;



    while(true)
    {
        // int data;
        // c_bq->pop(&data);
        // cout<<"消费者任务完成..."<<data<<endl;
        CallTask t;
        p_bq->pop(&t);
        cout<< "cal thread, 完成计算任务: "<<t()<<endl;
        //sleep(1);

        // SaveTask s(t(),Save);
        // c_bq->push(s);
        // cout<< "cal thread, 推送存储任务完成..." <<t()<<endl;

    }
    return nullptr;
}

void* saver(void* args)
{
    BlockQueue<SaveTask>* c_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->s_bq;

    while(true)
    {
        SaveTask s;
        c_bq->pop(&s);
        s();
        cout<<"save thread, 保存任务完成..."<<endl;
    }
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    //BlockQueue<int>* bq=new BlockQueue<int>();
    //BlockQueue<Task>* bq=new BlockQueue<Task>();
    BlockQueues<CallTask,SaveTask>* bq=new BlockQueues<CallTask,SaveTask>();
    bq->c_bq=new BlockQueue<CallTask>();
    bq->s_bq=new BlockQueue<SaveTask>();

    pthread_t c[2],p[2];
    pthread_create(p,nullptr,producer,bq);
    pthread_create(p+1,nullptr,producer,bq);
    //pthread_create(p+2,nullptr,producer,bq);
    pthread_create(c,nullptr,consumer,bq);
    pthread_create(c+1,nullptr,consumer,bq);



    // pthread_t c,p,s;
    // pthread_create(&c,nullptr,producer,bq);
    // pthread_create(&p,nullptr,consumer,bq);
    // pthread_create(&s,nullptr,saver,bq);


    // pthread_join(p,nullptr);
    // pthread_join(c,nullptr);
    // pthread_join(s,nullptr);

    pthread_join(p[0],nullptr);
    pthread_join(p[1],nullptr);
    //pthread_join(p[2],nullptr);
    pthread_join(c[0],nullptr);
    pthread_join(c[1],nullptr);

    delete bq->c_bq;
    delete bq->s_bq;

    return 0;
}


