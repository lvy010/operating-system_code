#pragma once
#include<iostream>
#include<pthread.h>

using namespace std;

class Mutexx
{
public:
    Mutexx(pthread_mutex_t* lock_p=nullptr)
        :_lock_p(lock_p)
    {}

    void lock()
    {
        pthread_mutex_lock(_lock_p);
    }

    void unlock()
    {
        pthread_mutex_unlock(_lock_p);
    }

    ~Mutexx()
    {}

private:
    pthread_mutex_t* _lock_p;
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* lock_p)
        :_mutex(lock_p)
    {
        _mutex.lock();
    }

    ~LockGuard()
    {
        _mutex.unlock();
    }
private:
    Mutexx _mutex;
};