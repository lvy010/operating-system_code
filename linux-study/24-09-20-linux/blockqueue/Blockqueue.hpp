#pragma once
#include<iostream>
#include<pthread.h>
#include<queue>

using namespace  std;

#define maxcapacity 5

template<class T>
class BlockQueue
{
public:
    BlockQueue(int capacity=maxcapacity)
        :_capacity(capacity)
    {
        pthread_mutex_init(&_lock,nullptr);
        pthread_cond_init(&_pcond,nullptr);
        pthread_cond_init(&_ccond,nullptr);
    }


    void push(const T& in)//生产者
    {
        pthread_mutex_lock(&_lock);
        while(is_full())//bug?
        {
            pthread_cond_wait(&_pcond,&_lock);//?
        }
        //走到这里一定没有满
        _q.push(in);
        pthread_cond_signal(&_ccond);
        pthread_mutex_unlock(&_lock);
    }

    void pop(T* out)//消费者
    {
        pthread_mutex_lock(&_lock);
        if(is_empty())
        {
            pthread_cond_wait(&_ccond,&_lock);
        }
        *out=_q.front();
        _q.pop();
        pthread_cond_signal(&_pcond);
        //pthread_cond_broadcast(&_pcond);
        pthread_mutex_unlock(&_lock);

    }

    ~BlockQueue()
    {
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_pcond);
        pthread_cond_destroy(&_ccond);
    }

private:
    bool is_empty()
    {
        return _q.empty();
    }

    bool is_full()
    {
        return _q.size() == _capacity;
    }


private:
    queue<T> _q;
    int _capacity;
    pthread_mutex_t _lock;
    pthread_cond_t _pcond;
    pthread_cond_t _ccond;
};