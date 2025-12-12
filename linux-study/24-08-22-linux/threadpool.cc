#include"ThreadPool.hpp"
#include<unistd.h>

int main()
{
    //ThreadPool<Task>* tp=new ThreadPool<Task>();
    //tp->run();
    //获取单例直接run起来
    ThreadPool<Task>::getInstance()->run();

    int x,y;
    char op;
    while(true)
    {
        cout<<"请输入第一个数据#";
        cin>>x;
        cout<<"请输入第二个数据#";
        cin>>y;
        cout<<"请输入要进行的操作#";
        cin>>op;
        Task t(x,y,op,mymath);
        ThreadPool<Task>::getInstance()->push(t);
        sleep(1);
    }

    return 0;
}