#pragma once

#include "protocol.hpp"

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
#include <functional>

using namespace std;

enum
{
    USAGG_ERR = 1,
    SOCKET_ERR,
    BIND_ERR,
    LISTEN_ERR
};

const int backlog = 5;

typedef function<void(const httpRequest&,httpResponse&)> func_t;

void handlerEntery(int sock,func_t callback)
{
    //1.读取
    //2.反序列化
    //3.处理
    //4.序列化
    //5.发送
    char buffer[4096];
    httpRequest req;
    httpResponse resp;
    ssize_t n=recv(sock,buffer,sizeof(buffer)-1,0);
    if(n>0)
    {
        buffer[n]=0;
        req.inbuffer=buffer;
        req.parse();
        callback(req,resp);

        send(sock,resp.outbuffer.c_str(),resp.outbuffer.size(),0);

    }
    else
    {
        return;
    }


}



class httpServer
{
public:
    httpServer(const uint16_t port) : _port(port), _listensock(-1)
    {
    }

    void initServer()
    {
        // 1.创建socket文件套接字对象
        _listensock = socket(AF_INET, SOCK_STREAM, 0);
        if (_listensock < 0)
        {
            exit(SOCKET_ERR);
        }

        // 2.bind 绑定自己的网络消息 port和ip
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY; // 任意地址bind,服务器真实写法

        if (bind(_listensock, (struct sockaddr *)&local, sizeof(local)) < 0)
        {
            exit(BIND_ERR);
        }


        // 3.设置socket为监听状态
        if (listen(_listensock, backlog) < 0) // backlog  底层链接队列的长度
        {
            exit(LISTEN_ERR);
        }

    }

    void start(func_t func)
    {
        // 子进程退出自动被OS回收
        signal(SIGCHLD, SIG_IGN);
        for (;;)
        {
            // 4.获取新链接
            struct sockaddr_in peer;
            socklen_t len = (sizeof(peer));
            int sock = accept(_listensock, (struct sockaddr *)&peer, &len); // 成功返回一个文件描述符
            if (sock < 0)
            {
                continue;
            }

            // 5.通信   这里就是一个sock,未来通信我们就用这个sock,tcp面向字节流的,后序全部都是文件操作!

            // version2 多进程信号版
            int fd = fork();
            if (fd == 0)
            {
                close(_listensock);
                handlerEntery(sock,func);
                close(sock);
                exit(0);
            }
            close(sock);
        }
    }

    ~httpServer()
    {
    }

private:
    // string _ip;
    uint16_t _port;
    int _listensock;
};
