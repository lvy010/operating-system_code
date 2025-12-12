#include<iostream>
#include<stdlib.h>
using namespace std;

int main()
{
    //系统自带环境变量
    printf("PATH:%s\n",getenv("PATH"));
    printf("PWD:%s\n",getenv("PWD"));
    iout<<"hello C++"<<endl;
    //自定义环境变量
    printf("MYENV:%s\n",getenv("MYENV"));
    cout<<"hello C++"<<endl;
    cout<<"hello C++"<<endl;
    cout<<"hello C++"<<endl;
    cout<<"hello C++"<<endl;
    cout<<"hello C++"<<endl;
    cout<<"hello C++"<<endl;
    return 0;
}
