#include"comm.hpp"


int main()
{
    cout<<"receive begin"<<endl;
    int rfd=open(NAME_PIPE,O_RDONLY);
    cout<<"receive end"<<endl;
    if(rfd < 0)
        exit(1);

    char buffer[1024];
    while(true)
    {
        ssize_t n=read(rfd,buffer,sizeof(buffer)-1);
        if(n>0)
        {
            buffer[n]=0;
            cout<<"send->receive# :"<<buffer<<endl;
        }
        else if(n == 0)
        {
            cout<<"send quit , Me too"<<endl;
            break;;
        }
        else
        {
            cout<<"error string: "<<strerror(errno)<<endl;
            break;
        }

    }
    close(rfd);


    return 0;
}