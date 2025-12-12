#pragma once

#include <iostream>
#include <sys/epoll.h>
#include <functional>
#include "sock.hpp"
#include "err.hpp"
#include "log.hpp"

class epollServer
{
    static const int defaultport = 8080;
    static const int defaultvalue = -1;
    static const int size = 64;
    static const int num = 128;

    using func_t = std::function<std::string(const std::string)>;

public:
    epollServer(func_t f, int port = defaultport) : _cbs(f), _port(port), _listensock(defaultvalue), _epfd(defaultvalue), _revs(nullptr)
    {
    }

    void initServer()
    {
        // 1.创建套接字
        _listensock = Sock::sock();
        Sock::Bind(_listensock, _port);
        Sock::Listen(_listensock);

        // 2.创建epoll模型
        _epfd = epoll_create(size);
        if (_epfd < 0)
        {
            logMessage(FATAL, "epoll create error: %s", strerror(errno));
            exit(EPOLL_CREATE_ERR);
        }

        // 3.将目前唯一_listensock,添加到epoll
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = _listensock; // 当事件就绪，被重新捞取上来的时候，我们要知道是哪一个fd就绪了！
        int n = epoll_ctl(_epfd, EPOLL_CTL_ADD, _listensock, &ev);
        if (n < 0)
        {
            logMessage(WARNING, "Add fail");
        }

        // 4. 申请就绪事件的空间
        _revs = new struct epoll_event[num];
    }

    void handlerevents(int readynum)
    {
        logMessage(DUGNUM,"handlerevents in");

        for(int i=0;i<readynum;++i)
        {
           
            uint32_t event=_revs[i].events; //获取就绪的事件
            int sock=_revs[i].data.fd;// 知道是谁的就绪事情,因此需要在添加的就设置这个参数

            //1.listen事件就绪,获取新连接
            if(sock == _listensock && (event & EPOLLIN))
            {
                string clientip;
                uint16_t clientport;
                int sock=Sock::Accept(_listensock,&clientip,&clientport);
                if(sock<0)
                { 
                    logMessage(WARNING,"accept error");
                    return;
                }

                // 获取fd成功，可以直接读取吗？？
                //不可以，IO=等+数据拷贝,你怎么知道有数据了,没有数据就会被阻塞! 只有epoll清楚
                //添加新sock到epoll
                //这里也不需要遍历数组找到合适位置了,直接交给内核就行了
                struct epoll_event ev;
                ev.events=EPOLLIN;
                ev.data.fd=sock;
                epoll_ctl(_epfd,EPOLL_CTL_ADD,sock,&ev);
            }
            else if(event & EPOLLIN)//普通事件就绪
            {
                // 依旧有问题
                char buffer[1024];
                // 把本轮数据读完，就一定能够读到一个完整的请求吗？？  不能! Reactor在处理
                int n=read(sock,buffer,sizeof(buffer)-1);
                if(n>0)
                {
                    buffer[n]=0;
                    logMessage(NORMAL,"client# %s",buffer);

                    std::string response=_cbs(buffer);

                    //写 也有问题
                    write(sock,response.c_str(),response.size());
                }
                else if(n==0)
                {
                    //注意细节,一定先在epoll模型删掉该fd,不然先close,epoll就找不到该fd了然后报错
                    epoll_ctl(_epfd,EPOLL_CTL_DEL,sock,nullptr);//不管什么事件了
                    close(sock);
                    logMessage(NORMAL,"client quit");
                }
                else
                {
                    epoll_ctl(_epfd,EPOLL_CTL_DEL,sock,nullptr);
                    close(sock);
                    logMessage(WARNING,"read error");
                }
            }
        }
        logMessage(DUGNUM,"handlerevents out");
    }

    void start()
    {
        int timeout=-1000;
        for (;;)
        {
            int n=epoll_wait(_epfd,_revs,num,timeout);
            switch(n)
            {
                case 0:
                logMessage(NORMAL,"timeout...");
                break;
                case -1:
                logMessage(WARNING, "epoll_wait failed, code: %d, errstring: %s", errno, strerror(errno));
                break;
                default:
                logMessage(NORMAL,"event readly");
                handlerevents(n);
                break;
            }
        }
    }

    ~epollServer()
    {
        if (_listensock != defaultvalue)
            close(_listensock);
        if (_epfd != defaultvalue)
            close(_epfd);
        if (_revs)
            delete[] _revs;
    }

private:
    int _listensock;
    int _port;
    int _epfd;                 // epoll模型fd
    struct epoll_event *_revs; // 拷贝数组
    func_t _cbs;               // 回调函数
};
