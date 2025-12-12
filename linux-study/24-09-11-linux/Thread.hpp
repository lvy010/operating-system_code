#pragma once
#include<iostream>
#include<string>
#include<pthread.h>
#include<functional>

class Thread
{
    typedef  std::function<void*(void*)> func_t;
private:
    //类内成员有隐藏的this指针,不加static就会报错!
    //但是我们又需要this指针,调用类的成员变量,因此把this传过来
    static void* start_routine(void* args)
    {
        Thread* _this=static_cast<Thread*>(args);//安全进行类型转换
        return _this->_func(_this->_args);//调用回调函数,不这样写也可以再写一个类内函数在调用
    }
public:
    Thread()
    {
        char namebuffer[64];
        snprintf(namebuffer,sizeof namebuffer,"thread-%d",_number++);
        _name=namebuffer;
    }

    //为什么这里参数不放在构造函数
    //因为我们等会想线程运行的时候,知道是那个线程在运行把_name也一起传过去
    void start(func_t func,void* args)
    {
        _func=func;
        _args=args;
        //这个函数不认识C++的function类,因此我自己写一个函数
        pthread_create(&_tid,nullptr,start_routine,this);
    }

    void join()
    {
        pthread_join(_tid,nullptr);
    }

    std::string threadname()
    {
        return _name;
    }

    ~Thread()
    {}

private:
    std::string _name;//线程名
    func_t _func;//回调函数
    void* _args;//回调函数参数
    pthread_t _tid;//线程ID

    static int _number;
};

int Thread::_number=1;


