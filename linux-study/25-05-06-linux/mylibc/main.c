#include"myStdio.h"


int main()
{
    _FILE* fp=_fopen("log.txt","w");
    if(fp == NULL)
    {
        perror("_fopen");
        return 1;
    }

    int cnt=10;
    const char* msg="hello linux";
    while(1)
    {
        _fwrite(msg,strlen(msg),fp);
        _fflush(fp);
        sleep(1);
        printf("count:%d\n",cnt--);
        if(cnt == 0) break;
    }
    _fclose(fp);

    return 0;
}
