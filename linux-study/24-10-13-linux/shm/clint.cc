#include"com.hpp"


int main()
{
    key_t k=GetKey();
    int shmid=GetShm(k);

    char* start=(char*)AttachShm(shmid);

    while(true)
    {
        printf("%s\n",start);
        sleep(2);
    }

    RemoveAttach(start);


    return 0;
}