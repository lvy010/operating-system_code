#pragma onec
#include<iostream>
#include<queue>
#include<pthread.h>

using namespace std;

const int maxcapacity=5;
 
template<class T>
class BlockQueue
{
public:
    BlockQueue(const int& capacity=maxcapacity)
        :_capacity(capacity)
    {
        //构造时初始化
        pthread_mutex_init(&_mutex,nullptr);
        pthread_cond_init(&_pcond,nullptr);
        pthread_cond_init(&_ccond,nullptr);
    }

    //生产者放数据
    void push(const T& in)//输入型参数,const &
    {
        //放之前先加锁保护共享资源,在加锁和解锁之间就是安全的临界资源
        pthread_mutex_lock(&_mutex);
        //1.判满
        while(is_full())//bug?
        {
            //因为生产条件不满足,无法生产,此时我们的生产者进行等待
            pthread_cond_wait(&_pcond,&_mutex);//_muext?
        }
        //2.走到这里一定是没有满
        _q.push(in);
        //3.绝对能保证,阻塞队列里面一定有数据
        pthread_cond_signal(&_ccond);//唤醒消费者,这里可以有一定策略,比如说满足三分之一在唤醒
        pthread_mutex_unlock(&_mutex);

    }

    //消费者拿数据
    void pop(T* out)//输出型参数,* //输入输出型 &
    {
        //这里也要加锁,因为要保证访问同一份资源是安全,所以用的是同一把锁
        pthread_mutex_lock(&_mutex);
        //1.判空
        while(is_empty())
        {
            pthread_cond_wait(&_ccond,&_mutex);
        }
        //2.走到这里我们能保证,一定不为空
        *out=_q.front();
        _q.pop();
        //3.绝对能保证,阻塞队列里面至少有一个空的位置
        pthread_cond_signal(&_pcond);//这里可以有一定策略
        pthread_mutex_unlock(&_mutex);
    }

    ~BlockQueue()
    {
        //析构时销毁
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_pcond);
        pthread_cond_destroy(&_ccond);
    }

private:
    bool is_full()
    {
        return _q.size()==_capacity;
    }

    bool is_empty()
    {
        return _q.empty();
    }

private:
    queue<T> _q;
    int _capacity;//不能让阻塞队列无限扩容,所以给一个最大容量表示队列的上限
    pthread_mutex_t _mutex;//阻塞队列是一个共享资源,所以需要一把锁把它保护起来
    //生产者对应的条件变量
    pthread_cond_t _pcond;//队列满了,一定要让生产者在对应的条件变量下休眠
    //消费者对应的条件变量
    pthread_cond_t _ccond;//队列空了,让消费者也在对应条件变量下休眠
};