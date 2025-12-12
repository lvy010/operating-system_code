#pragma once
#include<iostream>
#include<semaphore.h>
#include<pthread.h>
#include<vector>
#include<cassert>

using namespace std;

const int maxcapacity=10;

template<class T>
class ringqueue
{
public:

    ringqueue(int capacity=maxcapacity)
        :_capacity(capacity)
        ,_queue(capacity)
    {
        sem_init(&_spacesem,0,_capacity);
        sem_init(&_datasem,0,0);
        pthread_mutex_init(&_plock,nullptr);
        pthread_mutex_init(&_clock,nullptr);


        _prodstep=_constep=0;
    }

    void push(const T& in)
    {
        P(_spacesem);
        pthread_mutex_lock(&_plock);
        _queue[_prodstep++]=in;
        _prodstep%=_capacity;
        pthread_mutex_unlock(&_plock);
        V(_datasem);
    }

    void pop(T* out)
    {
        P(_datasem);
        pthread_mutex_lock(&_clock);
        *out=_queue[_constep++];
        _constep%=_capacity;
        pthread_mutex_unlock(&_clock);
        V(_spacesem);
    }

    ~ringqueue()
    {
        sem_destroy(&_spacesem);
        sem_destroy(&_datasem);
        pthread_mutex_destroy(&_plock);
        pthread_mutex_destroy(&_clock);

    }

private:
    void P(sem_t& sem)
    {
        int n=sem_wait(&sem);
        assert(n==0);
        (void)n;
    }

    void V(sem_t& sem)
    {
        int n=sem_post(&sem);
        assert(n==0);
        (void)n;
    }

private:
    vector<T> _queue;
    int _capacity;
    sem_t _spacesem;
    sem_t _datasem;
    int _prodstep;
    int _constep;
    pthread_mutex_t _plock;
    pthread_mutex_t _clock;
};