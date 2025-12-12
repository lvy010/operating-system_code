#include<iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<cassert>
#include<cerrno>
#include<cstring>

#define PATH_NAME "myfile.txt"
using namespace std;


int Createpipe(const char* pathname)
{
    umask(0);
    int n=mkfifo(pathname,0666);
    if(n<0)
    {
        cerr<<errno<<":"<<strerror(errno)<<endl;
        exit(1);
    }
    return 0;
}



void deletepipe(const char* pathname)
{
    int n=unlink(pathname);
    assert(n==0);
}