#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<assert.h>
#include<stdlib.h>
int main()
{
    pid_t id=fork();
    assert(id!=-1);
    if(id == 0)
    {
        int cnt=5;
        while(cnt)
        {
            printf("i am child process,cnt=%d\n",cnt--);
            sleep(1);
        }
        exit(12);
    }
    int status=0;
    pid_t ret=waitpid(id,&status,0);
    assert(ret>0);
    printf("wait success,exitsig=%d,exitCode=%d\n",status&0x7F,(status>>8)&0xFF);
    return 0;
}
