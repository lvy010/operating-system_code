#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <string>

using namespace std;

int ticket = 1000;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


void *GetTicket(void *args)
{
    string name = static_cast<const char *>(args);


    while (true)
    {
        pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond, &lock); //?
        if (ticket > 0)
        {
            usleep(1000);
            cout<<name<<" 正在进行抢票: "<<ticket<<endl;
            ticket--;
            pthread_mutex_unlock(&lock);
        }
        else
        {
            pthread_mutex_unlock(&lock);
            break;
        }
    }
}

int main()
{
    pthread_t t[5];
    for (int i = 0; i < 5; i++)
    {
        char *name = new char[64];
        snprintf(name, 64, "thread %d", i + 1);
        pthread_create(t+i, nullptr, GetTicket, name);
    }

    while (true)
    {
        sleep(1);
        pthread_cond_signal(&cond);
        std::cout << "main thread wakeup one thread..." << std::endl;
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_join(t[i], nullptr);
    }

    return 0;
}