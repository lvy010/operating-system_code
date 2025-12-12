#include<iostream>
#include<unistd.h>
#include<cstring>
#include<cassert>
#include<cstdlib>
#include<sys/types.h>
#include<sys/wait.h>

using namespace std;

int main()
{
    int pipefd[2];
    int n=pipe(pipefd);
    assert(n == 0);
    (void)0;

    pid_t fd=fork();
    if(fd == 0)
    {
        close(pipefd[1]);
        char buffer[1024];
        while(true)
        {
            ssize_t s=read(pipefd[0],buffer,sizeof buffer);
            if(s>0) 
            {
                buffer[s]=0;
                cout<<buffer<<endl;
            }
            else if(s==0)
            {
                cout<<"read->0"<<endl;
                break;
            }
        }
        close(pipefd[0]);
        exit(0);
    }

    close(pipefd[0]);
    char buffer[1024];
    int cnt=0;
    const char* s="I am father";
    while(true)
    {
        
        snprintf(buffer,sizeof buffer,"%s([%d]->[%d])",s,cnt++,getpid());
        ssize_t k=write(pipefd[1],buffer,strlen(buffer));
        assert(k>0);
        sleep(2);
    }
    
    close(pipefd[1]);

    waitpid(fd,nullptr,0);

}