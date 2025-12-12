#pragma once

#include<iostream>
#include<pthread.h>
#include<vector>
#include<semaphore.h>
#include<cassert>

using namespace std;

const int maxcapacity=5;

template<class T>
class ringqueue
{
public:
    ringqueue(int capacity=maxcapacity):_queue(capacity),_capacity(capacity)
    {
        //空间资源初始化为环形队列大小
        int n=sem_init(&_spacesem,0,_capacity);
        assert(n == 0);
        //数据资源初始化为0
        n=sem_init(&_datasem,0,0);
        assert(n == 0);
        pthread_mutex_init(&_plock,nullptr);
        pthread_mutex_init(&_clock,nullptr);

        _pstep=_cstep=0;
    }

    void push(const T& in)
    {
        //1.申请空间资源P操作, 成功意味着我一定能进行正常的生产,失败阻塞挂起
        P(_spacesem);
        pthread_mutex_lock(&_plock);
        //2.往对应生产下标处生产
        _queue[_pstep++]=in;
        _pstep%=_capacity;//为了是一个环形的
        //3.环形队列多了一个消费资源
        pthread_mutex_unlock(&_plock);
        V(_datasem);  
    }

    void pop(T* out)
    {
        P(_datasem);//申请成功,意味着一定能进行正常的消费
        pthread_mutex_lock(&_clock);
        *out=_queue[_cstep++];//从对应的消费下标处消费
        _cstep%=_capacity;
        pthread_mutex_unlock(&_clock);
        V(_spacesem);//环形队列多了一个空间资源

    }

    ~ringqueue()
    {
        //销毁
        sem_destroy(&_spacesem);
        sem_destroy(&_datasem);
        pthread_mutex_destroy(&_plock);
        pthread_mutex_destroy(&_clock);
    }
private:
    void P(sem_t& sem)//对信号量做--
    {
        int n=sem_wait(&sem);
        assert(n == 0);
        (void)n;
    }

    void V(sem_t& sem)//对信号量做++
    {
        int n=sem_post(&sem);
        assert(n == 0);
        (void)n;
    }


private:
    vector<T> _queue;//模拟环形队列
    int _capacity;//队列的大小,不能无线扩容
    sem_t _spacesem;//生产者生产看中的空间资源(信号量)
    sem_t _datasem;//消费者消费看中的数据资源(信号量)
    int _pstep;//生产者下标
    int _cstep;//消费者下标
    pthread_mutex_t _plock;//生产者的锁
    pthread_mutex_t _clock;//消费者的锁
};