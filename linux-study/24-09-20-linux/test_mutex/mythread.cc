#include "Thread.hpp"
#include <unistd.h>
#include <vector>
#include "Mutex.hpp"

int ticket = 10000;

// pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

class Mutex
{
public:
    Mutex(pthread_mutex_t *mutex_p = nullptr, string name = "")
        : _mutex_p(mutex_p), _name(name)
    {
    }

public:
    pthread_mutex_t *_mutex_p;
    string _name;
};

void *GetTicket(void *args)
{
    // string name=static_cast<const char*>(args);
    Mutex *mut = static_cast<Mutex *>(args);
    while (true)
    {
        {
            LockGuard lockgurad(mut->_mutex_p);
            // pthread_mutex_lock(&lock);
            // pthread_mutex_lock(mut->_mutex_p);
            // pthread_mutex_lock(mut->_mutex_p);
            if (ticket > 0)
            {
                usleep(1000);
                // cout<<name<<" 正在进行抢票: "<<ticket<<endl;
                cout << mut->_name << " 正在进行抢票: " << ticket << endl;
                ticket--;
                // pthread_mutex_unlock(mut->_mutex_p);
                // pthread_mutex_unlock(&lock);
            }
            else
            {
                // pthread_mutex_unlock(mut->_mutex_p);
                // pthread_mutex_unlock(&lock);
                break;
            }
        }
        // usleep(1000);
        // 一般抢完票并不是直接结束
        usleep(1000);
    }
}

int main()
{
    // Thread t1(GetTicket,(void*)"thread->1",1);
    // Thread t2(GetTicket,(void*)"thread->2",2);
    // Thread t3(GetTicket,(void*)"thread->3",3);
    // Thread t4(GetTicket,(void*)"thread->4",4);

    // t1.join();
    // t2.join();
    // t3.join();
    // t4.join();

    pthread_mutex_t lock;

    pthread_mutex_init(&lock, nullptr);

#define NUM 4
    vector<pthread_t> tids(NUM);
    for (int i = 0; i < NUM; ++i)
    {
        char namebuffer[64];
        snprintf(namebuffer, sizeof(namebuffer), "thread %d", i + 1);
        Mutex *mut = new Mutex(&lock, namebuffer);
        pthread_create(&tids[i], nullptr, GetTicket, mut);
    }

    for (auto &thread : tids)
    {
        pthread_join(thread, nullptr);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}