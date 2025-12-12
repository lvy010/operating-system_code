#pragma once

#include <iostream>
#include <functional>
#include <poll.h>
#include "sock.hpp"

using namespace std;

class pollServer
{
    static const int defaultport = 8080;
    static const int defaultfd = -1;
    static const int defaultnum = 2048;

    using func_t = function<string(string)>;

public:
    pollServer(func_t f, int port = defaultport) : _cbs(f), _port(port), _listensock(-1), _rfds(nullptr)
    {
    }

    void initServer()
    {
        // 1.创建套接字
        _listensock = Sock::sock();
        Sock::Bind(_listensock, _port);
        Sock::Listen(_listensock);

        _rfds = new struct pollfd[defaultnum]; // 大小这里自己随便定
        for (int i = 0; i < defaultnum; ++i)    ResetItem(i);
        _rfds[0].fd = _listensock; // 这个位置后面就不变了
        _rfds[0].events = POLLIN;  // 告诉内核帮我关心_listensock读事件
    }

    void print()
    {
        for (int i = 0; i < defaultnum; ++i)
        {
            if (_rfds[i].fd != defaultfd)
                cout << _rfds[i].fd << " ";
        }
        cout << endl;
    }

    void Accepter(int listensock)
    {

        logMessage(DEBUG, "Accepter in");

        // 走到这里,accept 函数,会不会被阻塞?

        // 走到这里就是, poll 告送我,_listensock就绪了,然后才能执行下面代码

        string clientip;
        uint16_t clientport;
        int sock = Sock::Accept(listensock, &clientip, &clientport); //// accept = 等 + 获取
        if (sock < 0)
            return;
        logMessage(NORMAL, "accept success [%s:%d]", clientip.c_str(), clientport);

        // 得到一个sock套接字后，然后我们可以直接进行read/recv吗? 不能,整个代码只有poll有资格检测事件是否就绪
        // 将新的sock 托管给poll！
        // 将新的sock，托管给select的本质，其实就是将sock，添加到fdarray数组里！
        int i = 0;
        for (; i < defaultnum; ++i)
        {
            if (_rfds[i].fd != defaultfd)
                continue;
            else
                break;
        }
        if (i == defaultnum)
        {
            logMessage(WARNING, "server if full, please wait");
            close(sock);
        }
        else
        {
            _rfds[i].fd = sock;
            _rfds[i].events = POLLIN;
            _rfds[i].revents = 0;
        }

        print();
        logMessage(DEBUG, "Accepter out");
    }

    void ResetItem(int i)
    {
        _rfds[i].fd = defaultfd;
        _rfds[i].events = 0;
        _rfds[i].revents = 0;
    }

    void Recver(int pos)
    {
        logMessage(DEBUG, "in Recver");

        // 1. 读取request
        // 这样读取是有问题的！
        char buffer[1024];
        ssize_t s = recv(_rfds[pos].fd, buffer, sizeof(buffer) - 1, 0); // 这里在进行读取的时候，会不会被阻塞？
        if (s > 0)                                                      // 读取成功
        {
            buffer[s] = 0;
            logMessage(NORMAL, "client# %s", buffer);
        }
        else if (s == 0) // 对方关闭了文件描述符
        {
            close(_rfds[pos].fd);
            ResetItem(pos);
            logMessage(NORMAL, "client quit");
            return;
        }
        else // 读取失败
        {
            close(_rfds[pos].fd);
            ResetItem(pos);
            logMessage(ERROR, "client quit: %s", strerror(errno));
            return;
        }

        // 2. 处理request
        std::string response = _cbs(buffer);

        // 3. 返回response
        // write bug
        write(_rfds[pos].fd, response.c_str(), response.size());

        logMessage(DEBUG, "out Recver");
    }

    // 1.handler event _rfds 中,不仅仅是有一个fd是就绪的,可能存在多个
    // 2.我们的poll目前只处理了read事件
    void HandlerEvent()
    {
        // 你怎么知道那些fd就绪了呢? 我不知道,我只能遍历
        for (int i = 0; i < defaultnum; ++i)
        {
            // 不合法fd
            if (_rfds[i].fd == defaultfd)
                continue;

            // 合法fd,但必须曾经向内核设置过帮我关心对应fd读事件才能往下走
            if (!(_rfds[i].events & POLLIN))
                continue;

            if (_rfds[i].fd == _listensock && _rfds[i].revents & POLLIN)
                Accepter(_listensock);
            else if (_rfds[i].revents & POLLIN)
                Recver(i);
        }
    }

    void start()
    {
        int timenout = -1;
        for (;;)
        {
            int n = poll(_rfds, defaultnum, timenout);
            switch (n)
            {
            case 0:
                logMessage(NORMAL, "timeout...");
                break;
            case -1:
                logMessage(WARNING, "poll error, code: %d, err string: %s", errno, strerror(errno));
                break;
            default:
                // 说明有事件就绪了,目前只有一个监听事件就绪了
                logMessage(NORMAL, "have event ready!");
                HandlerEvent(); // 这里不用传了,因为就绪事件就在_rfds里
                break;
            }
        }
    }

    ~pollServer()
    {
        if (_listensock != defaultfd)
            close(_listensock);
        if (_rfds)
            delete[] _rfds;
    }

private:
    int _listensock;
    int _port;
    struct pollfd *_rfds;
    func_t _cbs;
};