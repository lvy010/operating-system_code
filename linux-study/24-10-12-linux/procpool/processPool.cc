#include<iostream>
#include<vector>
#include<unistd.h>
#include<cassert>
#include<cstdlib>
#include<ctime>
#include<sys/types.h>
#include<sys/wait.h>

#define Pro_NUM 5
using namespace std;


///////////////////////////////////////////////////////任务/////////////////////////////
//func_t 是一个函数指针 ，typedef之后这是一个函数指针类型
typedef void(*func_t)();

void DownloadTask()
{
    cout<<"下载任务"<<endl;
    sleep(1);
}

void  IOTask()
{
    cout<<"IO任务"<<endl;
    sleep(1);
}

void FFlushTask()
{
    cout<<"刷新任务"<<endl;
    sleep(1);
}

void LoadTask(vector<func_t>& ff)
{
    assert(&ff != nullptr);
    ff.push_back(DownloadTask);
    ff.push_back(IOTask);
    ff.push_back(FFlushTask);
}

///////////////////////////////////////下面是一个多进程///////////////////////////
class subEP
{
public:
    subEP(pid_t subfd,int writefd)
        :_subfd(subfd)
        ,_writefd(writefd)
    {
        char buffer[1024];
        snprintf(buffer,sizeof buffer,"process->%d[pid(%d)-fd(%d)]",cnt++,_subfd,_writefd);
        _name=buffer;
    }

public:
    static int cnt;
    string _name;
    pid_t _subfd;
    int _writefd;

};

int subEP::cnt=0;

int recTask(int readFd)
{
    int code=0;
    ssize_t n=read(readFd,&code,sizeof code);
    assert(n == sizeof(int));
    (void)n;
    if(n == 4) 
        return code;
    else if (n <= 0)
        return -1;
    else
        return 0;
    
}

void  CreateSubProcess(vector<subEP>& sub,vector<func_t> func)
{
    for(int i=0;i<Pro_NUM;++i)
    {
        //创建管道
        int pipefd[2];
        int n=pipe(pipefd);
        assert(n == 0);
        (void)n;
        //创建子进程
        pid_t fd=fork();
        if(fd == 0)
        {
            //子进程
            close(pipefd[1]);//关闭写
            //子进程处理任务
            while(true)
            {
                int Commande=recTask(pipefd[0]);
                if(Commande >= 0 && Commande < func.size())
                    func[Commande]();
                else if(Commande == -1)//读到文件结尾
                    break;
            }
            exit(0);
     
        }
        //关闭读
        close(pipefd[0]);
        subEP ss(fd,pipefd[1]);
        sub.push_back(ss);   
    }
}

void SendTask(const subEP& process,int tasknum)
{
    cout<<"send task num: "<<tasknum<<"send to: "<<process._name<<endl;
    //父进程发的是4字节的任务码
    ssize_t n=write(process._writefd,&tasknum,sizeof tasknum);
    assert(n == sizeof(int));
    (void)n;

}

void loadBlanceContrl(vector<subEP>& sub,vector<func_t>& func,int& count)
{
    int processnum=sub.size();
    int funcnum=func.size();
    bool flage=(count == 0? true:false);
    //让每个子进程都可能被选，因此加一个随机数
    while(true)
    {
        //选某个子进程
        int subidx=rand()%processnum;
        //选某个任务
        int taskidx=rand()%funcnum;
        //发送任务
        SendTask(sub[subidx],taskidx);
        sleep(2);
        if(!flage)
        {
            count--;
            if(count == 0)
                break;           
        }
    }

    //走到这里关闭写 write quit,read->0
    for(int i=0;i<processnum;++i)
    {
        close(sub[i]._writefd);
    }

}

void waitprocess(vector<subEP> sub)
{
    for(int i=0;i<sub.size();++i)
    {
        waitpid(sub[i]._subfd,nullptr,0);
        cout << "wait sub process success ...: " << sub[i]._subfd << endl;
    }
}

int main()
{
    //生成随机数种子
    srand((unsigned int)time(nullptr));
    //创建任务对象
    vector<func_t> funMap;
    //上传任务
    LoadTask(funMap);
    //创建子进程，并维护好父子间通信信道
    vector<subEP> subs;
    CreateSubProcess(subs,funMap);
    //走到这里是父进程，控制子进程，负载均衡的向子进程发送命令码
    int count=5;
    loadBlanceContrl(subs,funMap,count);
    //回收子进程资源
    waitprocess(subs);

    return 0;
}