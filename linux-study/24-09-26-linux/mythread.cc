#include "Thread.hpp"
#include <unistd.h>
#include <vector>
#include "Mutex.hpp"

int ticket = 10000;

// 全局锁
// pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

// 又想把"thread->1"参数给GetTicket，也想把锁给它
class ThreadData
{
public:
    ThreadData(pthread_mutex_t *mutex_p, const string &name)
        : _mutex_p(mutex_p), _threadname(name)
    {
    }
    ~ThreadData()
    {
    }

public:
    pthread_mutex_t *_mutex_p;
    string _threadname;
};

void *GetTicket(void *args)
{
    // string name = static_cast<const char *>(args);
    ThreadData *td = static_cast<ThreadData *>(args);
    while (true)
    {
        {
            LockGuard lockguard(td->_mutex_p);
            // 加锁
            // pthread_mutex_lock(td->_mutex_p);
            // pthread_mutex_lock(td->_mutex_p);//申请一次，再申请一次
            if (ticket > 0)
            {
                usleep(1000);
                // cout << name<< " 正在进行抢票: " << ticket << endl;
                cout << td->_threadname << " 正在进行抢票: " << ticket << endl;
                ticket--;
                // 以微秒为单位进行休眠，模拟真实的抢票要花费的时间
                // usleep(1000);
                // pthread_mutex_unlock(td->_mutex_p);//解锁
            }
            else
            {
                // 加锁可能条件不满足走到else，这里也需要解锁
                // pthread_mutex_unlock(td->_mutex_p);
                break;
            }
        }

        // 模拟处理其他事情
        usleep(1000);
    }
}

int main()
{

    // 局部锁
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, nullptr); // 使用前初始化

#define NUM 4
    vector<pthread_t> tids(NUM);
    for (int i = 0; i < NUM; ++i)
    {
        char namebuffer[64];
        snprintf(namebuffer, sizeof(namebuffer), "thread->%d", i + 1);
        ThreadData *td = new ThreadData(&lock, namebuffer);
        pthread_create(&tids[i], nullptr, GetTicket, td);
    }

    for (auto &thread : tids)
    {
        pthread_join(thread, nullptr);
    }

    pthread_mutex_destroy(&lock);

    // Thread t1(GetTicket,(void*)"thread->1",1);
    // Thread t2(GetTicket,(void*)"thread->2",2);
    // Thread t3(GetTicket,(void*)"thread->3",3);
    // Thread t4(GetTicket,(void*)"thread->4",4);

    // t1.join();
    // t2.join();
    // t3.join();
    // t4.join();

    return 0;
}

