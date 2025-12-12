#include<iostream>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<cerrno>
#include<cstring>
#include<cstdlib>

using namespace std;

#define PATH_NAME "."
#define PRO_OBJ  0x123
#define MAX_SIZE 4096

key_t GetKey()
{
    key_t k=ftok(PATH_NAME,PRO_OBJ);
    if(k==-1)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return k;
}

int getShmid(key_t k,int flage)
{
    int shmid=shmget(k,MAX_SIZE,flage);
    if(shmid==-1)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return shmid;
}

int CreateShm(key_t k)
{
    return getShmid(k,IPC_CREAT|IPC_EXCL|0666);
}

int GetShm(key_t k)
{
    return getShmid(k,IPC_CREAT);
}


void* AttachShm(int shmid)
{
    void* mem=shmat(shmid,nullptr,0);
    if((long long)mem == -1L)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return mem;
}


void RemoveAttach(const void* mem)
{
    if(shmdt(mem) == -1)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
}


void DeleteShm(int shmid)
{
    if(shmctl(shmid,IPC_RMID,nullptr) == -1)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
}