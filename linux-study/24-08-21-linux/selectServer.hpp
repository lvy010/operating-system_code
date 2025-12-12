#pragma once

#include <iostream>
#include<functional>
#include "sock.hpp"

using namespace std;

class SelectServer
{
    static const int defaultport = 8080;
    static const int fdnum = sizeof(fd_set) * 8;
    static const int defaultfd = -1;

    using func_t=function<string(string)>;

public:
    SelectServer(func_t f,int port = defaultport) : _cbs(f),_port(port), _listensock(-1), _fdarray(nullptr)
    {
    }

    void initServer()
    {
        // 1.创建套接字
        _listensock = Sock::sock();
        Sock::Bind(_listensock, _port);
        Sock::Listen(_listensock);

        _fdarray = new int[fdnum];
        for (int i = 0; i < fdnum; ++i)
            _fdarray[i] = defaultfd;
        _fdarray[0] = _listensock; // 这个位置后面就不变了
    }

    void print()
    {
        for (int i = 0; i < fdnum; ++i)
        {
            if (_fdarray[i] != defaultfd)
                cout << _fdarray[i] << " ";
        }
        cout << endl;
    }

    void Accepter(int listensock)
    {

        logMessage(DEBUG, "Accepter in");

        // 走到这里,accept 函数,会不会被阻塞?

        // 走到这里就是, select 告送我,_listensock就绪了,然后才能执行下面代码

        string clientip;
        uint16_t clientport;
        int sock = Sock::Accept(listensock, &clientip, &clientport); //// accept = 等 + 获取
        if (sock < 0)
            return;
        logMessage(NORMAL, "accept success [%s:%d]", clientip.c_str(), clientport);

        // 得到一个sock套接字后，然后我们可以直接进行read/recv吗? 不能,整个代码只有select有资格检测事件是否就绪
        // 将新的sock 托管给select！
        // 将新的sock，托管给select的本质，其实就是将sock，添加到fdarray数组里！
        int i = 0;
        for (; i < fdnum; ++i)
        {
            if (_fdarray[i] != defaultfd)
                continue;
            else
                break;
        }
        if (i == fdnum)
        {
            logMessage(WARNING, "server if full, please wait");
            close(sock);
        }
        else
        {
            _fdarray[i] = sock;
        }

        print();
        logMessage(DEBUG, "Accepter out");
    }

    void Recver(int sock, int pos)
    {
        logMessage(DEBUG, "in Recver");

        // 1. 读取request
        // 这样读取是有问题的！
        char buffer[1024];
        ssize_t s = recv(sock, buffer, sizeof(buffer) - 1, 0); // 这里在进行读取的时候，会不会被阻塞？
        if (s > 0)//读取成功
        {
            buffer[s] = 0;
            logMessage(NORMAL, "client# %s", buffer);
        }
        else if (s == 0) //对方关闭了文件描述符
        {
            close(sock);//我也关
            _fdarray[pos] = defaultfd;//不让select关心该sock了
            logMessage(NORMAL, "client quit");
            return;
        }
        else//读取失败
        {
            close(sock);
            _fdarray[pos] = defaultfd;
            logMessage(ERROR, "client quit: %s", strerror(errno));
            return;
        }

        // 2. 处理request
        std::string response = _cbs(buffer);

        // 3. 返回response
        // write bug
        write(sock, response.c_str(), response.size());

        logMessage(DEBUG, "out Recver");
    }

    // 1.handler event rfds 中,不仅仅是有一个fd是就绪的,可能存在多个
    // 2.我们的select目前只处理了read事件
    void HandlerEvent(fd_set &rfds)
    {
        // 你怎么知道那些fd就绪了呢? 我不知道,我只能遍历
        for (int i = 0; i < fdnum; ++i)
        {
            // 不合法fd
            if (_fdarray[i] == defaultfd)
                continue;

            // 合法fd,但不一定就绪,要先判断
            if (_fdarray[i] == _listensock && FD_ISSET(_listensock, &rfds))
                Accepter(_listensock);
            else if (FD_ISSET(_fdarray[i], &rfds))
                Recver(_fdarray[i], i);
        }
    }

    void start()
    {

        for (;;)
        {
            fd_set rfds;
            FD_ZERO(&rfds); // 对读文件描述符集初始化
            int maxfd = _fdarray[0];
            for (int i = 0; i < fdnum; ++i)
            {
                if (_fdarray[i] == defaultfd) // 非法,没有被设置的
                    continue;

                // 因为rfds是输入输出型参数,因此每次都要将合法fd,重新添加到读文件描述符集
                FD_SET(_fdarray[i], &rfds);
                if (maxfd < _fdarray[i])
                    maxfd = _fdarray[i];
            }
            // FD_SET(_listensock, &rfds); // 将_listensock添加到读文件描述符集合中
            //  struct timeval timeout = {3, 0};
            //   我告诉select关心读文件描述符集中的_listensock事件,就绪了之后告诉我
            //   int n = select(_listensock + 1, &rfds, nullptr, nullptr, &timeout);
            int n = select(maxfd + 1, &rfds, nullptr, nullptr, nullptr);
            switch (n)
            {
            case 0:
                logMessage(NORMAL, "timeout...");
                break;
            case -1:
                logMessage(WARNING, "select error, code: %d, err string: %s", errno, strerror(errno));
                break;
            default:
                // 说明有事件就绪了,目前只有一个监听事件就绪了
                logMessage(NORMAL, "have event ready!");
                HandlerEvent(rfds);

                break;
            }
        }
    }

    ~SelectServer()
    {
        if (_listensock < 0)
            close(_listensock);
        if (_fdarray)
            delete[] _fdarray;
    }

private:
    int _listensock;
    int _port;
    int *_fdarray;
    func_t _cbs;

};