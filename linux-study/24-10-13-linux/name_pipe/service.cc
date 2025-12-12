#include "comm.hpp"

int main()
{
    int rfd=open(PATH_NAME,O_RDONLY);

    while(true)
    {
        char buffer[1024];
        ssize_t n=read(rfd,buffer,sizeof(buffer)-1);
        if(n>0) 
        {
            buffer[n]=0;
            cout<<"Get Message#"<<buffer<<endl;
        }
        else if(n == 0)
        {
            cout<<"read->0"<<endl;
            break;

        }
    }
    close(rfd);
    return 0;
}