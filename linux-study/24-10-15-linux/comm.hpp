#include<iostream>
#include<cerrno>
#include<cstring>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>

using namespace std;

#define PATHNAME "."
#define PROJ_JD  0x11

#define MAX_SIZE 4096


key_t GetKey()
{
    key_t k=ftok(PATHNAME,PROJ_JD);
    if(k < 0)
    {
        //cin cout cerr->stdin stdout stderr(默认打开的三个文件)-(fd)>0,1,2->键盘，显示器，显示器
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return k;
}

//获得共享内存表示符shmid
int getShmHelper(int key,int flage)
{
    int shmid=shmget(key,MAX_SIZE,flage);
    if(shmid < 0)
    {    
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return shmid;
}

//写端创建共享内存
int CreateShm(key_t key)
{
    return getShmHelper(key,IPC_CREAT|IPC_EXCL|0666);
}

//读端获取共享内存
int GetShm(key_t key)
{
    return getShmHelper(key,IPC_CREAT);
}


void* attachShm(int shimid)
{
    void* mem=shmat(shimid,nullptr,0);//linux 64位机器指针大小位9
    if((long long)mem == -1L)//1L代表是长整型
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return mem;
}

void detachShm(const void* adder)
{
    if(shmdt(adder) == -1)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
}

//删除共享内存
void DelShm(int shmid)
{
    if(shmctl(shmid,IPC_RMID,nullptr) == -1)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
    }

}