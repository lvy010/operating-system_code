#pragma once
#include "Thread.hpp"
#include "Task.hpp"
#include <vector>
#include <queue>
#include "Mutex.hpp"
#include <mutex>

using namespace std;
const int maxcap = 3;

// 声明
template <class T>
class ThreadPool;

template <class T>
class ThreadData
{
public:
    ThreadData(ThreadPool<T> *poolthis, const string &name) : _poolthis(poolthis), _name_(name)
    {
    }
    ~ThreadData()
    {
    }

public:
    ThreadPool<T> *_poolthis;
    string _name_;
};

template <class T>
class ThreadPool
{
private:
    // 线程调用的处理任务函数
    static void *handTask(void *args)
    {
        ThreadData<T> *td = static_cast<ThreadData<T> *>(args);

        while (true)
        {
            Task t;
            // RAII 风格加锁
            {
                // 构造时自动加锁,析构时自动结束
                // 局部变量生命周期这个代码块
                LockGuard lockguard(td->_poolthis->mutex());
                while (td->_poolthis->IsQueueEmpty())
                {
                    td->_poolthis->threadwait();
                }
                td->_poolthis->pop(&t);
            }
            // cout << td->_name_ << " 处理完了任务: " << t() << endl;
            t();

            // //这里我们写了一些函数调用,也可以每个都加this指针调用

            // //放任务之前加锁
            // td->_poolthis->threadlock();
            // while(td->_poolthis->IsQueueEmpty())//任务队列空线程就等待
            // {
            //     td->_poolthis->threadwait();
            // }
            // //取任务
            // Task t;
            // td->_poolthis->pop(&t);
            // //注意一定要先解锁,在处理任务!不然串行处理任务一点意义都没有!!
            // td->_poolthis->threadunlock();
            // //线程并行处理任务
            // cout<<td->_name_<<" 处理完了任务: "<<t()<<endl;
        }
        delete td;
        return nullptr;
    }

private:
    void threadlock() { pthread_mutex_lock(&_lock); }
    void threadunlock() { pthread_mutex_unlock(&_lock); }
    void threadwait() { pthread_cond_wait(&_cond, &_lock); }
    void pop(T *out)
    {
        *out = _task_queue.front();
        _task_queue.pop();
    }
    bool IsQueueEmpty() { return _task_queue.empty(); }
    pthread_mutex_t *mutex()
    {
        return &_lock;
    }

    // 单例不是没有例,构造函数不能去掉,放在private就好了
    ThreadPool(int cap = maxcap) : _cap(maxcap)
    {
        // 初始化锁,条件变量
        pthread_mutex_init(&_lock, nullptr);
        pthread_cond_init(&_cond, nullptr);

        // 创建线程
        for (int i = 0; i < _cap; ++i)
        {
            _threads.push_back(new Thread()); // 创建线程并放在vector里
        }
    }

    // 去掉赋值,拷贝构造
    void operator=(const ThreadPool &) = delete;
    ThreadPool(const ThreadPool &) = delete;

public:
    // 启动线程
    // 在Thread里说过,想把线程名也传过去,但是回调函数只有一个函数
    // 而这函数我们写在类里面必须要加一个static,导致没有this指针,而使用类内成员需要this指针
    // 因此我们写个类把线程名和this都传过去
    void run()
    {
        for (auto &thread : _threads)
        {
            ThreadData<T> *td = new ThreadData<T>(this, thread->threadname());
            thread->start(handTask, td);
            //cout << thread->threadname() << " statr... " << endl;
        }
    }

    // 任务队列放任务
    void push(const T &in)
    {
        // 保证放任务是安全的,所以先加锁
        pthread_mutex_lock(&_lock);
        _task_queue.push(in);
        pthread_cond_signal(&_cond); // 队列中有任务就唤醒等待的线程去取任务
        pthread_mutex_unlock(&_lock);
    }

    ~ThreadPool()
    {
        // 销毁锁,条件变量
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_cond);
    }

    // 获取单例
    // 成员函数可以调用静态成员和静态成员函数,反之不行
    static ThreadPool<T> *getInstance()
    {
        // 虽然没有并发问题了,但是还有一个小问题
        // 未来每一个线程进来都要lock,unlock
        // 因此在外面再加一个if判断,未来只要第一次实例化之后就不需要再加锁解锁了
        // 大家就可以并发了
        if (tp == nullptr)
        {
            _singlock.lock();
            if (tp == nullptr)
            {
                tp = new ThreadPool<T>();
            }
            _singlock.unlock();
        }

        return tp;
    }

private:
    int _cap;                  // 线程个数
    vector<Thread *> _threads; // 线程放在vector里进行管理
    queue<T> _task_queue;      // 任务队列
    pthread_mutex_t _lock;
    pthread_cond_t _cond;

    static ThreadPool<T> *tp;
    // c++11的锁
    static std::mutex _singlock;
};

template <class T>
ThreadPool<T> *ThreadPool<T>::tp = nullptr;

template <class T>
mutex ThreadPool<T>::_singlock;



