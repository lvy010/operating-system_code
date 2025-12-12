#pragma once

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>

using namespace std;

void SetNOBlock(int fd)
{
    int fl=fcntl(fd,F_GETFL);
    if(fl<0)
    {
       cerr << "fcntl : " << strerror(errno) << endl;
       return;
    }
    fcntl(fd,F_SETFL,fl|O_NONBLOCK);
}


void printLog()
{
    cout<<"this is a log"<<endl;
}

void download()
{
    cout<<"this is a download"<<endl;
}

void executeSql()
{
    cout<<"this is a executeSql"<<endl;
}
