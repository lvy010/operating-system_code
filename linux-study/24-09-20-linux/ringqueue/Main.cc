#include"ringqueue.hpp"
#include<ctime>
#include<sys/types.h>
#include<unistd.h>
#include"Task.hpp"

template<class P,class C>
struct ringqueues
{
    ringqueue<P>* _prq;
    ringqueue<C>* _crq;
};



void* productor(void* args)
{
    //ringqueue<int>* _rq=static_cast<ringqueue<int>*>(args);
    //ringqueue<CallTask>* _rq=(static_cast<ringqueues<CallTask,SaveTask>*>(args))->_prq;
    ringqueue<CallTask>* _rq=static_cast<ringqueue<CallTask>*>(args);


    while(true)
    {
        // int data=rand()%100+1;
        // _rq->push(data);
        int x=rand()%10+1;
        int y=rand()%5;
        char op=str[rand()%str.size()];
        CallTask t(x,y,op,mymath);
        _rq->push(t);
        cout<<"produetor，完成生产任务："<<t.toTaskString()<<endl;
        sleep(1);
    }
    return nullptr;
}

void* consumer(void* args)
{
    //ringqueue<int>* _rq=static_cast<ringqueue<int>*>(args);
    //ringqueue<CallTask>* p_rq=(static_cast<ringqueues<CallTask,SaveTask>*>(args))->_prq;
    //ringqueue<SaveTask>* c_rq=(static_cast<ringqueues<CallTask,SaveTask>*>(args))->_crq;
    ringqueue<CallTask>* _rq=static_cast<ringqueue<CallTask>*>(args);


    while(true)
    {
        // int data;
        // _rq->pop(&data);
        CallTask t;
        //p_rq->pop(&t);
        _rq->pop(&t);
        cout<<"consumer，完成消费任务: "<<t()<<endl;


        // SaveTask s(t(),Save);
        // c_rq->push(s);
        // cout<<"consumer，完成存储任务: "<<t()<<endl;
        //sleep(1);
    }
    return nullptr;
}

void* saver(void* args)
{
    ringqueue<SaveTask>* rq=(static_cast<ringqueues<CallTask,SaveTask>*>(args))->_crq;

    while(true)
    {
        SaveTask s;
        rq->pop(&s);
        s();
        cout<<"consumer,保存任务完成"<<endl;
    }
    return nullptr;
}



int main()
{
    srand((unsigned int)time(nullptr)^getpid());
    //ringqueue<int> *rq=new ringqueue<int>();

    // ringqueues<CallTask,SaveTask>* rq=new ringqueues<CallTask,SaveTask>();
    // rq->_prq=new ringqueue<CallTask>();
    // rq->_crq=new ringqueue<SaveTask>();

    ringqueue<CallTask>* rq=new ringqueue<CallTask>();

    //pthread_t p,c,s;
    // pthread_create(&p,nullptr,productor,rq);
    // pthread_create(&c,nullptr,consumer,rq);
    //pthread_create(&s,nullptr,saver,rq);
    pthread_t p[8],c[4];
    for(int i=0;i<8;++i)
    {
        pthread_create(p+i,nullptr,productor,rq);
    }
    
    for(int i=0;i<4;++i)
    {
        pthread_create(c+i,nullptr,consumer,rq);
    }


    // pthread_join(p,nullptr);
    // pthread_join(c,nullptr);
   //pthread_join(s,nullptr);

    for(int i=0;i<8;++i)
    {
        pthread_join(p[i],nullptr);
    }
    
    for(int i=0;i<4;++i)
    {
        pthread_join(c[i],nullptr);
        
    }

    return 0;
}