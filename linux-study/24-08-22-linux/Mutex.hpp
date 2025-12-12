#pragma once
#include<iostream>
#include<pthread.h>

class Mutex
{
public:
    Mutex(pthread_mutex_t* lock):_lock(lock)
    {
        pthread_mutex_init(_lock,nullptr);
    }

    void lock()
    {
        pthread_mutex_lock(_lock);
    }

    void unlock()
    {
        pthread_mutex_unlock(_lock);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(_lock);
    }
private:
    pthread_mutex_t* _lock;
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* lock):_mutex(lock)
    {
        _mutex.lock();
    }
    ~LockGuard()
    {
        _mutex.unlock();
    }
private:
    Mutex _mutex;
};