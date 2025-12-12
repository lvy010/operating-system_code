#pragma once
#include<iostream>
#include<pthread.h>
#include<string>
#include<functional>
#include<cstdio>
#include<cassert>

using namespace std;

//声明
class Thread;

class Context
{
public:
    Thread* _this;
    void* args_;

    Context()
        :_this(nullptr)
        ,args_(nullptr)
    {}
};

class Thread
{
public:
    typedef function<void*(void*)> func_t;
    const int num=1024;
public:


    static void* start_routine(void* args)//类内函数，有缺省参数(this指针)
    {
        Context* tr=static_cast<Context*>(args);
        tr->_this->_func(tr->args_);
        //静态函数只能调用静态成员和静态函数
        //return _func(_args);
    }

    Thread(func_t func,void* args=nullptr,int number=0)
        :_func(func)
        ,_args(args)
    {
        char namebuffer[num];
        snprintf(namebuffer,sizeof(namebuffer),"thread->%d",number);
        _name=namebuffer;

        Context* tr=new Context();
        tr->_this=this;
        tr->args_=_args;

        //调用回调函数
        int n=pthread_create(&_tid,nullptr,start_routine,tr);
        // int n=pthread_create(&_tid,nullptr,start_routine,_args);
        assert(n == 0);
        (void)n;
    }

    void join()
    {
        int n=pthread_join(_tid,nullptr);
        assert(n==0);
        (void)n;
    }

    ~Thread()
    {}

private:
    string _name;
    func_t _func;//回调函数
    void* _args;//回调函数参数
    pthread_t _tid;//线程ID
};