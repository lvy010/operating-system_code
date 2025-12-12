#include"comm.hpp"

int main()
{
    key_t key=GetKey();
    printf("key->%d\n",key);
    int shmid=CreateShm(key);
    printf("shimid->%d\n",shmid);

    //关联
    char* start=(char*)attachShm(shmid);//我想将这个地址认为是一字符串;
    printf("attach success, address start: %p\n", start);
    
    //写
    char buffer[1024];
    const char* messge="hello clint,我是另一个进程,我正在和你通信";
    int id=getpid();
    int cnt=0;
    while(true)
    {
        snprintf(start,MAX_SIZE,"%s([%d]->[%d])",messge,cnt++,id);
        sleep(5);
        //以往我们的做法
        // snprintf(buffer,sizeof buffer,"%s([%d]->[%d])",messge,cnt++,id));
        // memcpy(start,buffer,strlen(buffer)+1);
    }

    //去关联
    detachShm(start);

    //删除
    DelShm(shmid);

    return 0;
}