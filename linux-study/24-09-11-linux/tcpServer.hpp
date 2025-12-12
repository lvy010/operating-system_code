#pragma once

#include "logMessage.hpp"
#include"ThreadPool.hpp"
#include"Task.hpp"

#include <iostream>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include<pthread.h>

using namespace std;

enum
{
    USAGG_ERR = 1,
    SOCKET_ERR,
    BIND_ERR,
    LISTEN_ERR
};

const int backlog = 5;
class tcpServer;//声明

struct ThreadDate
{
    ThreadDate(int sock,tcpServer* tps)
        :_sock(sock),_tps(tps)
    {}

    int _sock;
    tcpServer* _tps;
};


class tcpServer
{
public:
    tcpServer(const uint16_t port) : _port(port), _listensock(-1)
    {
    }

    void initServer()
    {
        // 1.创建socket文件套接字对象
        _listensock = socket(AF_INET, SOCK_STREAM, 0);
        if (_listensock < 0)
        {
            logMessage(FATAL, "socket create error");
            exit(SOCKET_ERR);
        }
         logMessage(NORMAL, "socker create success :%d",_listensock);
        //logMessage(NORMAL, "socker create success");


        // 2.bind 绑定自己的网络消息 port和ip
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY; // 任意地址bind,服务器真实写法

        if (bind(_listensock, (struct sockaddr *)&local, sizeof(local)) < 0)
        {
            logMessage(FATAL, "bind socket error");
            exit(BIND_ERR);
        }
        logMessage(NORMAL, "bind socket success");

        // 3.设置socket为监听状态
        if (listen(_listensock, backlog) < 0) // backlog  底层链接队列的长度
        {
            logMessage(FATAL, "listen socket error");
            exit(LISTEN_ERR);
        }
        logMessage(NORMAL, "listen socker success");
    }

    // static void* start_routine(void* args)
    // {
    //     ThreadDate* td=static_cast<ThreadDate*>(args);
    //     pthread_detach(pthread_self());//退出自动回收资源
    //     td->_tps->serverIO(td->_sock);
    //     close(td->_sock);
    //     delete td;
    //     td=nullptr;
    // }


    void start()
    {
        //线程池启动
        ThreadPool<Task>::getInstance()->run();
        logMessage(NORMAL, "Thread init success");

        //子进程退出自动被OS回收
        //signal(SIGCHLD,SIG_IGN);
        for (;;)
        {
            // 4.获取新链接
            struct sockaddr_in peer;
            socklen_t len = (sizeof(peer));
            int sock = accept(_listensock, (struct sockaddr *)&peer, &len); // 成功返回一个文件描述符
            if (sock < 0)
            {
                logMessage(ERROR, "accpet error");
                continue;
            }
             logMessage(NORMAL, "accpet a new link success,get new sock: %d",sock);
            //logMessage(NORMAL, "accpet a new link success,get new sock");
            //cout << "sock: " << sock << endl;

            // 5.通信   这里就是一个sock,未来通信我们就用这个sock,tcp面向字节流的,后序全部都是文件操作!

            // version 1
            //  serverIO(sock);
            //  close(sock);//关闭文件,不然会造成文件描述符泄漏

            // version2 多进程
            //  int fd=fork();
            //  if(fd == 0) //child
            //  {
            //      close(_listensock);
            //      if(fork() > 0) exit(0);//创建孙子进程,让子进程退出,孙子进程变成孤儿进程被OS领养
            //      serverIO(sock);
            //      close(sock);
            //      exit(0);
            //  }
            //  //close(sock);
            //  pid_t ret=waitpid(fd,nullptr,0);
            //  if(ret > 0)
            //  {
            //      logMessage(NORMAL,"waitpid child success");
            //  }

            // version2 多进程信号版
            // int fd=fork();
            // if(fd == 0)
            // {
            //     close(_listensock);
            //     serverIO(sock);
            //     close(sock);
            //     exit(0);
            // }
            // close(sock);


            //version3  多线程
            // pthread_t pid;
            // ThreadDate* td=new ThreadDate(sock,this);
            // pthread_create(&pid,nullptr,start_routine,td);


            //version4 线程池
            ThreadPool<Task>::getInstance()->push(Task(sock,serverIO));
        }
    }


    // void serverIO(int &sock)
    // {
    //     char buffer[1024];
    //     while (true)
    //     {
    //         // 读
    //         ssize_t n = read(sock, buffer, sizeof(buffer));
    //         if (n > 0)
    //         {
    //             buffer[n] = 0;
    //             cout << "recv message: " << buffer << endl;

    //             // 写
    //             string outbuffer = buffer;
    //             outbuffer += "server [respond]";
    //             write(sock, outbuffer.c_str(), outbuffer.size());
    //         }
    //         else if (n == 0)
    //         {
    //             // 代表clien退出
    //             logMessage(NORMAL, "client quit, me to!");
    //             break;
    //         }
    //     }
    // }

    ~tcpServer()
    {
    }

private:
    // string _ip;
    uint16_t _port;
    int _listensock;
};
