#include"com.hpp"


int main()
{
    key_t k=GetKey();
    int shmid=CreateShm(k);

    char* start=(char*)AttachShm(shmid);

    const char* s="I am process A";
    int cnt=0;
    while(true)
    {
        snprintf(start,MAX_SIZE,"%s(%d)",s,cnt++);
        sleep(1);
    }
    
    RemoveAttach(start);

    DeleteShm(shmid);
    return 0;
}