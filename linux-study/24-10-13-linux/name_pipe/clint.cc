#include"comm.hpp"

int main()
{
    int n=Createpipe(PATH_NAME);
    assert(n==0);

    int wfd=open(PATH_NAME,O_WRONLY|O_TRUNC);

    const char* s= "I am process A";
    char buffer[1024];
    int cnt=0;
    while(true)
    {
        snprintf(buffer,sizeof buffer,"%s([%d]->[%d])",s,cnt++,getpid());
        ssize_t s=write(wfd,buffer,strlen(buffer));
        assert(s>0);
        (void)s;
        sleep(1);
    }
    close(wfd);
    deletepipe(PATH_NAME);
    return 0;
}