#pragma once

#include "logMessage.hpp"
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

typedef function<void(const Request &, Response &)> func_t;

enum
{
    OK,
    DIV_ERR,
    MOD_ERR,
    OPER_ERR
};

void handlerEntery(int sock, func_t callback)
{
    string inbuffer;
    while (true)
    {
        string req_text, req_str;
        // 1. 读取："content_len"\r\n"x op y"\r\n
        // 1.1 你怎么保证你读到的消息是 【一个】完整的请求
        if (!recvpackge(sock, inbuffer, &req_text))
            return;

        cout << "带报头的请求：\n"
             << req_text << std::endl;

        /// 1.2 我们保证，我们req_text里面一定是一个完整的请求："content_len"\r\n"x op y"\r\n
        if (!Delenth(req_text, &req_str))
            return;

        cout << "去掉报头的正文：\n"
             << req_str << endl;

        // 2. 对请求Request，反序列化
        // 2.1 得到一个结构化的请求对象
        Request req;
        if (!req.deserialize(req_str))
            return;

        // 3. 计算机处理，req.x, req.op, req.y --- 业务逻辑
        // 3.1 得到一个结构化的响应
        Response resp;
        callback(req, resp); // req的处理结果，全部放入到了resp， 回调是不是不回来了？不是！

        // 4.对响应Response，进行序列化
        // 4.1 得到了一个"字符串"
        string resp_str;
        if (!resp.serialize(&resp_str))
            return;

        cout << "计算完成, 序列化响应: " << resp_str << endl;

        // 5. 然后我们在发送响应
        // 5.1 构建成为一个完整的报文
        string send_string = Enlenth(resp_str);

        cout << "构建完成完整的响应\n"<< send_string << endl;

        send(sock, send_string.c_str(), send_string.size(), 0);
    }
}

class CalServer
{
public:
    CalServer(const uint16_t port) : _port(port), _listensock(-1)
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
        logMessage(NORMAL, "socker create success :%d", _listensock);

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
                logMessage(ERROR, "accpet error");
                continue;
            }
            logMessage(NORMAL, "accpet a new link success,get new sock: %d", sock);

            // 5.通信   这里就是一个sock,未来通信我们就用这个sock,tcp面向字节流的,后序全部都是文件操作!

            // version2 多进程信号版
            int fd = fork();
            if (fd == 0)
            {
                close(_listensock);
                handlerEntery(sock, func);
                close(sock);
                exit(0);
            }
            close(sock);
        }
    }

    ~CalServer()
    {
    }

private:
    // string _ip;
    uint16_t _port;
    int _listensock;
};
