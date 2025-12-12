#pragma once
#include<iostream>
#include<pthread.h>

using namespace std;

class Mutex
{
public:
    Mutex(pthread_mutex_t* lock_p=nullptr)
        :_lock_p(lock_p)
    {}

    void lock()
    {
        if(_lock_p) pthread_mutex_lock(_lock_p);
    }

    void unlock()
    {
        if(_lock_p) pthread_mutex_unlock(_lock_p);
    }

    ~Mutex()
    {}

private:
    pthread_mutex_t* _lock_p;
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* lockp)
        :_mutex(lockp)
    {
        _mutex.lock();//在构造函数中进行加锁
    }
    
    ~LockGuard()
    {
        _mutex.unlock();//在析构函数中进行解锁
    }

private:
    Mutex _mutex;
};

