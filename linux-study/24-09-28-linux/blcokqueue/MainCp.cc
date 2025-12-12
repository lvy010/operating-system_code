#include"BlockQueue.hpp"
#include<ctime>
#include<unistd.h>
#include"Task.hpp"

//C 计算
//S 存储
template<class C,class S>
class BlockQueues
{
public:
    BlockQueue<C>* c_bq;
    BlockQueue<S>* s_bq;
};

void* productor(void* args)
{
    //BlockQueue<int>* bq=static_cast<BlockQueue<int>*>(args);
    //BlockQueue<Task>* bq=static_cast<BlockQueue<Task>*>(args);
    //BlockQueue<CallTask>* _c_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->c_bq;

    BlockQueue<CallTask>* _c_bq=static_cast<BlockQueue<CallTask>*>(args);


    while(true)
    {
        //生产活动
        //int data=rand()%10+1;//在这里先用随机数.构建一个数据
        //bq->push(data);
        //cout<<"生产数据: "<<data<<endl;
        // int x=rand()%10+1;
        // int y=rand()%5;
        // char op=oper[rand()%oper.size()];
        // Task t(x,y,op,mymath);
        // bq->push(t);
        // cout<<"生产任务: "<<t.toTaskString()<<endl;
        int x=rand()%10+1;
        int y=rand()%5;
        char op=oper[rand()%oper.size()];
        CallTask t(x,y,op,mymath);
        _c_bq->push(t);
        cout<<"productor thread, 生产计算任务: "<<t.toTaskString()<<endl; 

        sleep(1);//生产的慢一些
    }

}

void* consumer(void* args)
{
    //BlockQueue<int>* bq=static_cast<BlockQueue<int>*>(args);
    //BlockQueue<Task>* bq=static_cast<BlockQueue<Task>*>(args);
    // BlockQueue<CallTask>* _c_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->c_bq;
    // BlockQueue<SaveTask>* _s_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->s_bq;

    BlockQueue<CallTask>* _c_bq=static_cast<BlockQueue<CallTask>*>(args);



    while(true)
    {
        //消费活动
        // int data;
        // bq->pop(&data);
        //cout<<"消费数据: "<<data<<endl;
        // Task t;
        // bq->pop(&t);
        // cout<<"消费任务: "<<t()<<endl;

        CallTask t;
        _c_bq->pop(&t);
        cout<< "cal thread, 完成计算任务: "<<t()<<endl;

        // SaveTask s(t(),Save);
        // _s_bq->push(s);
        // cout<< "cal thread, 推送存储任务完成..." <<t()<<endl;

        //sleep(1);//消费的慢一些
    }

}

void* saver(void* args)
{
    BlockQueue<SaveTask>* _s_bq=(static_cast<BlockQueues<CallTask,SaveTask>*>(args))->s_bq;

    while(true)
    {
        SaveTask s;
        _s_bq->pop(&s);
        s();
        cout<<"save thread, 保存任务完成..."<<endl;
    }
}

int main()
{
    //随机数种子
    srand((unsigned int)time(nullptr));

    //BlockQueue<int>* bq=new BlockQueue<int>();
    //BlockQueue<Task>* bq=new BlockQueue<Task>();

    //写一个类把两个阻塞队列都传过去
    // BlockQueues<CallTask,SaveTask>* bq=new BlockQueues<CallTask,SaveTask>();
    // bq->c_bq=new BlockQueue<CallTask>();
    // bq->s_bq=new BlockQueue<SaveTask>();

    BlockQueue<CallTask>* bq=new BlockQueue<CallTask>();


    pthread_t p[3],c[2];
    for(int i=0;i<3;++i)
    {
        pthread_create(p+i,nullptr,productor,bq);
    }
    
    for(int i=0;i<2;++i)
    {
        pthread_create(c+i,nullptr,consumer,bq);
    }

    for(int i=0;i<3;++i)
    {
        pthread_join(p[i],nullptr);
    }
    
    for(int i=0;i<2;++i)
    {
        pthread_join(c[i],nullptr);
    }


    //pthread_t p,c,s;

    //两个线程看到同一个阻塞队列
    // pthread_create(&p,nullptr,productor,bq);
    // pthread_create(&c,nullptr,consumer,bq);
    //pthread_create(&s,nullptr,saver,bq);

    // pthread_join(p,nullptr);
    // pthread_join(c,nullptr);
    //pthread_join(s,nullptr);



    // delete bq->c_bq;
    // delete bq->s_bq;


    return 0;
}