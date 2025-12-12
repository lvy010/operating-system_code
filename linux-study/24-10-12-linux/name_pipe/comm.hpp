#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<cassert>
#include<cstring>
#include<unistd.h>
#include<cstring>

#define NAME_PIPE "mypipe.txt"
using namespace std;


//创建管道
bool Createpipe(const string& p)
{

    //这里我们想要创建出的文件的权限就是0666，
    umask(0);
    int n=mkfifo(p.c_str(),0666);
    if(n == 0)
    {
        return true;
    }
    else
    {
        cout<<"errno: "<<errno<<"err string: "<<strerror(errno)<<endl;
        return false;
    }
}




//删除管道
void RemovePipe(const string& p)
{
    int n=unlink(p.c_str());
    assert(n == 0);
    (void)n;

}