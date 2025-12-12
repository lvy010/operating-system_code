#include"comm.hpp"

int main()
{
    key_t key=GetKey();
    printf("key->%d\n",key);
    int shmid=GetShm(key);
    printf("shimid->%d\n",shmid);

    char* start=(char*)attachShm(shmid);
    printf("attach success, address start: %p\n", start);

    while(true)
    {
         printf("service say : %s\n", start);
         struct shmid_ds ds;
         shmctl(shmid,IPC_STAT,&ds);
         printf("获得属性: size :%d,link :%d,key :%d\n",\
                ds.shm_segsz,ds.shm_nattch,ds.shm_perm.__key);
         sleep(1);
    }

    return 0;
}