#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Err.hpp"
#include "Log.hpp"

using namespace std;

class Sock
{
    const static int backlog = 32;
    const static int defaultsock = -1;

public:
    Sock(int sock = defaultsock) : _listensock(sock)
    {}

    ~Sock()
    {
        if (_listensock != defaultsock)
            close(_listensock);
    }

public:
    int sock()
    {
        // 1. 创建socket文件套接字对象
        _listensock= socket(AF_INET, SOCK_STREAM, 0);
        if (_listensock < 0)
        {
            logMessage(FATAL, "create socket error");
            exit(SOCKET_ERR);
        }
        logMessage(NORMAL, "create socket success: %d", _listensock);


        int opt = 1;
        setsockopt(_listensock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    }

    int Fd()
    {
        return _listensock;
    }

    void Bind(int port)
    {
        // 2. bind绑定自己的网络信息
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = INADDR_ANY;
        if (bind(_listensock, (struct sockaddr *)&local, sizeof(local)) < 0)
        {
            logMessage(FATAL, "bind socket error");
            exit(BIND_ERR);
        }
        logMessage(NORMAL, "bind socket success");
    }

    void Listen()
    {
        // 3. 设置socket 为监听状态
        if (listen(_listensock, backlog) < 0)
        {
            logMessage(FATAL, "listen socket error");
            exit(LISTEN_ERR);
        }
        logMessage(NORMAL, "listen socket success");
    }

    int Accept(string *clientip, uint16_t *clientport,int* err)
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        int sock = accept(_listensock, (struct sockaddr *)&peer, &len);
        *err=errno;
        if (sock < 0){}
            //logMessage(ERROR, "accept error, next");
        else
        {
            //logMessage(NORMAL, "accept a new link success, get new sock: %d", sock); // ?
            *clientip = inet_ntoa(peer.sin_addr);
            *clientport = ntohs(peer.sin_port);
        }
        return sock;
    }

private:
    int _listensock;
};