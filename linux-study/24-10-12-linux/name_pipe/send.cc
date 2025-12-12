#include"comm.hpp"



int main()
{
    bool flage=Createpipe(NAME_PIPE);
    assert(flage);
    (void)flage;

    cout<<"send begin"<<endl;
    int wfd=open(NAME_PIPE,O_WRONLY);
    cout<<"send end"<<endl;
    if(wfd<0)   
        exit(1);

    char buffer[1024];
    while(true)
    {
        cout<<"Play Say# ";
        fgets(buffer,sizeof buffer,stdin);
        if(strlen(buffer) > 0)
            buffer[strlen(buffer)-1]=0;
        ssize_t n=write(wfd,buffer,strlen(buffer));
        assert(n == strlen(buffer));
        (void)0;
    }

    close(wfd);
    
    RemovePipe(NAME_PIPE);
    return 0;

}